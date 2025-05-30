#include "scan_request_dto.h"
#include "http_client.h"
#include "async_inference.hpp"
#include "utils.hpp"
#include <thread>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <unistd.h>
#include <vector>
#include <filesystem>   // C++17 ile klasör yoksa oluşturmak için
#include <fstream>   // <-- missing
#include <sstream>   // if you keep to_json()
#include <iomanip>  
#include <utility> 

#include <vector>
#include "image_interface.h"
#include "udp_sender.hpp"
#include "Object_info.hpp"
#include "system_status_dto.h"
#include "system_messages_dto.h"
#include "HttpServerHandler.hpp"

// mert arduino flush variables başlangıç
inline static const std::string InoFilePath = "../SerialPort_communication/SerialPort_communication.ino";
inline static const std::string ARDUINO_PORT {"/dev/ttyUSB0"};
// mert arduino flush variables bitiş

pthread_barrier_t sync_barrier;



const std::string LOG_FILE = "../obj_det_stats.log";

std::atomic_bool keep_logging{true};

/////////// Constants ///////////
constexpr size_t MAX_QUEUE_SIZE = ImageInterface::QUEUE_SIZE;
/////////////////////////////////

std::shared_ptr<BoundedTSQueue<PreprocessedFrameItem>> preprocessed_queue =
    std::make_shared<BoundedTSQueue<PreprocessedFrameItem>>(MAX_QUEUE_SIZE);

std::shared_ptr<BoundedTSQueue<InferenceOutputItem>>   results_queue =
    std::make_shared<BoundedTSQueue<InferenceOutputItem>>(MAX_QUEUE_SIZE);
    
std::shared_ptr<BoundedTSQueue<SystemLogMessageDTO>>   system_message_queue =
    std::make_shared<BoundedTSQueue<SystemLogMessageDTO>>(MAX_QUEUE_SIZE);



std::atomic<bool> camera0_active{true};
std::atomic<bool> camera1_active{true};
std::atomic<bool> camera2_active{true};


#define CAMERAS ImageInterface::CAMERA_NUMBER

std::atomic_int active_cameras(CAMERAS); 

double threshold(70);

std::atomic_bool system_ready(false); // Seko delay için flag

std::atomic_bool all_cameras_done(false);

int count = ImageInterface::SAVE_NUMBER;

int num_camera = ImageInterface::CAMERA_NUMBER;



typedef struct CamBuf{
    int camId;
    cv::Mat frame;
    std::mutex m;
    bool object_detection = false;
    std::atomic_bool camera = false;
}CamBuf;




static std::unordered_map<int, std::chrono::steady_clock::time_point> last_capture_ts;


constexpr auto CAPTURE_COOLDOWN = std::chrono::seconds(ImageInterface::COOLDOWN_SECONDS);


struct CpuTimes { long long user, nice, system, idle; };


double get_cpu_temp() {
    std::ifstream t("/sys/class/thermal/thermal_zone0/temp");
    double millideg;
    if (t >> millideg) 
		return millideg / 1000.0;   // °C
	
	SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "get_cpu_temp failed");
	system_message_queue->push(msg);
    
    throw std::runtime_error("temp read failed");
}



CpuTimes read_proc_stat() {
    std::ifstream f("/proc/stat");
    std::string cpu;
    CpuTimes ct{};
    f >> cpu >> ct.user >> ct.nice >> ct.system >> ct.idle;
    return ct;
}

double get_cpu_usage(int ms = 500) {
    auto a = read_proc_stat();
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    auto b = read_proc_stat();

    long long idle   = b.idle  - a.idle;
    long long totalA = a.user + a.nice + a.system + a.idle;
    long long totalB = b.user + b.nice + b.system + b.idle;
    long long total  = totalB - totalA;

    return 100.0 * (total - idle) / total;  // %
}

struct MemInfo { long long free, total; };

MemInfo get_mem() {
    std::ifstream f("/proc/meminfo");
    std::string key, unit;
    long long val;
    MemInfo m{};
    while (f >> key >> val >> unit) {
        if (key == "MemTotal:") m.total = val;
        if (key == "MemAvailable:") m.free = val;
        if (m.free && m.total) break;
    }
    // kB → MiB
    m.total /= 1024; m.free /= 1024;
    return m;
}

void log_system_messages()
{
    // Initialize HTTP client
    HttpClient client;
    if (!client.initialize()) {
        std::cerr << "Failed to initialize HTTP client for system stats" << std::endl;
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to initialize HTTP client for system stats");
		system_message_queue->push(msg);
        return;
    }
    
    // Server configuration 
    std::string host = ImageInterface::SERVER_IP;
    int port = ImageInterface::BACKEND_PORT;
    std::string path = ImageInterface::BACKEND_SYSTEMMESSAGE_POINT;

    while (keep_logging) 
    {
		SystemLogMessageDTO logMessage;
        bool hasLog = system_message_queue->pop(logMessage);

        if (!hasLog) 
            continue;
        
        // Send system message to server
        bool success = client.sendSystemMessage(host, port, path, logMessage);
        
        if (success) 
        {
           std::cout << "başarılı" << std::endl; 
        }
        else 
        {
            std::cerr << "Failed to send system status to server" << std::endl;
            SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to send system status to server");
			system_message_queue->push(msg);
        }

        // Wait 5 seconds (same as original timing)
        for (int i = 0; i < 50 && keep_logging; ++i)    
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}



void log_system_stats()
{
    // Initialize HTTP client
    HttpClient client;
    if (!client.initialize()) {
        std::cerr << "Failed to initialize HTTP client for system stats" << std::endl;
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to initialize HTTP Client for system stats");
		system_message_queue->push(msg);
        return;
    }
    
    // Server configuration 
    std::string host = ImageInterface::SERVER_IP;
    int port = ImageInterface::BACKEND_PORT;
    std::string path = ImageInterface::BACKEND_SYSTEMINFO_POINT;

    while (keep_logging) 
    {
        /*  --- gather data --- */
        double temp   = get_cpu_temp();
        double cpuPct = get_cpu_usage();
        MemInfo mem   = get_mem();

        // Format memory usage as string
        std::ostringstream memStream;
        memStream << mem.free << "/" << mem.total << " MiB";
        std::string memoryUsage = memStream.str();
        
        // Create SystemStatusDTO with current timestamp
        SystemStatusDTO systemStatus(std::chrono::system_clock::now(), temp, cpuPct, memoryUsage);
        
        // Send system status to server
        bool success = client.sendSystemStatus(host, port, path, systemStatus);
        
        if (success) {
            std::cout << "System status sent: " << temp << "°C, " << cpuPct << "%, " << memoryUsage << std::endl;
        } 
        else {
            std::cerr << "Failed to send system status to server" << std::endl;
            SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to send system status to server");
			system_message_queue->push(msg);
        }

        // Wait 5 seconds (same as original timing)
        for (int i = 0; i < 50 && keep_logging; ++i)    
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/*
void log_system_stats()
{
    std::ofstream log(LOG_FILE, std::ios::app);
    if (!log) {
        std::cerr << "⚠️  Cannot open " << LOG_FILE << " for logging\n";
        return;
    }

    while (keep_logging) {
        //  --- gather data --- 
        double temp   = get_cpu_temp();
        double cpuPct = get_cpu_usage();
        MemInfo mem   = get_mem();

        std::time_t tt = std::time(nullptr);
        log << std::put_time(std::localtime(&tt), "%F %T") << ", "
            << std::fixed << std::setprecision(1)
            << temp << " °C, "
            << cpuPct << " %, "
            << mem.free << '/' << mem.total << " MiB free\n";

        log.flush();
        for (int i = 0; i < 50 && keep_logging; ++i)    
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
*/

std::pair<std::string,bool> parse_class_string(const std::string& full)
{
	const auto pos = full.find('_');
	if (pos == std::string::npos){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "String parser couldn't find '_'");
		system_message_queue->push(msg);
		return {full, true};                // _ yoksa 'sağlıklı' varsay
	}
	const std::string base = full.substr(0, pos);
	const std::string state = full.substr(pos + 1);
	const bool healthy = (state != "Rotten" && state != "rotten");
	return {base, healthy};
}





void release_resources(cv::VideoCapture &capture, cv::VideoWriter &video, InputType &input_type) {
    if (input_type.is_video) {
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Input type is video");
		system_message_queue->push(msg);
        video.release();
    }
    if (input_type.is_camera) {
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Input type is camera");
		system_message_queue->push(msg);
        capture.release();
        cv::destroyAllWindows();
    }
    preprocessed_queue->stop();
    results_queue->stop();
}

// seko deneme 
std::vector<std::string> class_names;

/*
// For defining 3 of the frames are HEALTHY OR ROTTEN (SEKO KOD)
std::string decide_class_of_frames(std::vector<std::string>& Cnames){
    std::string result;
    // All 3 frames' result are healthy, so return classname_healty
    if((Cnames[0] == Cnames[1] && Cnames[1] == Cnames[2]) && (Cnames[0].find("Healthy") != std::string::npos))
        result = Cnames[0];
    // One of them is Rotten, so total result must be classname_rotten
    else if((Cnames[0].find("Rotten") != std::string::npos))
        result = Cnames[0];
    else if((Cnames[1].find("Rotten") != std::string::npos))
        result = Cnames[1];
    else if((Cnames[2].find("Rotten") != std::string::npos))
        result = Cnames[2];
    
    while(Cnames.size() > 0)
        Cnames.pop_back();
    return result;    
}
*/

/*
// kapinin sağa ya da sola dönebilmesi icin prototip fonksiyon -- written by seko
void mekanik_kol(std::string::res){
    if(res.find("Healthy") != std::string::npos){
        // kolu sağa çevir?
    }
    else if(res.find("Rotten") != std::string::npos){
        // kolu sola çevir?
    }
}
*/
std::vector<ScanRequestDTO> scans;

bool isProductHealthy(const std::vector<ScanRequestDTO>& scans)
 {
    double score = 0.0;
    int numberOfScans = scans.size();
    std::unordered_map<std::string, double> productConfidenceMap;
	int count = 0;
	bool flag1 = false, flag2 = false;
    for (const auto& scan : scans) {
        double confidence = scan.confidence();
        std::string productResult = scan.productResult();

        std::string productId, result;
        std::istringstream iss(productResult);
        std::getline(iss, productId, '_');
        std::getline(iss, result);

        bool isSuccess = (result == "Healthy");
        if(isSuccess)
			count++;
        double health = isSuccess ? confidence : -1 * confidence;

        score += health / numberOfScans;

        // Sum health per product ID
        productConfidenceMap[productId] += health;
    }
	std::cout << "Health score is: " << score << "threshold is: " << threshold << std::endl;
	if(count == numberOfScans)
		flag1 = true;
	if(score >= threshold)
		flag2 = true;
		
	if(flag1 == true && flag2 == true) // tüm isimler "Healthy" ve score >= threshold
		return true;
	else if(flag1 == true && flag2 == false) // tüm isimler "Healthy" ama !(score >= threshold)
		return false; // muallakta kalındı
	else if(flag1 == false && flag2 == true) // tüm isimler "Healthy" degil(Rotten) ama score >= threshold
		return false;
	else if(flag1 == false && flag2 == false) // tüm isimler "Healthy" degil (Rotten) ve !(score >= threshold)
		return false;
	return false; // buraya gelmemesi bekleniyor
}

bool send_to_server(HttpClient& client, std::string& host,int& port, std::string& path){
	bool success = client.sendScans(host, port, path, scans);
    
		if (success) {
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Scans successfully sent to server");
			system_message_queue->push(msg);
			std::cout << "Scans successfully sent to server" << std::endl;
		} 
		else {
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to send scans to server");
			system_message_queue->push(msg);
			std::cerr << "Failed to send scans to server" << std::endl;
		}
		
		return isProductHealthy(scans);
}
 



hailo_status run_post_process(
    InputType &input_type,
    CommandLineArgs args,
    int org_height,
    int org_width,
    size_t frame_count,
    cv::VideoCapture &capture,
    ArduinoSerial &arduino,
    size_t class_count = 80,
    double fps = 30) 
    {

    cv::VideoWriter video;
    if (input_type.is_video || (input_type.is_camera && args.save)) {    
        init_video_writer("./processed_video.mp4", video, fps, org_width, org_height);
    }
    int i = 0;
    
    /* Htpp Client */
        HttpClient client;
    if (!client.initialize()) {
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to initialize HTTP CLient");
		system_message_queue->push(msg);
        std::cerr << "Failed to initialize HTTP client" << std::endl;
    }
     std::string host = ImageInterface::SERVER_IP;  // Change to your server's hostname or IP
    int port = ImageInterface::BACKEND_PORT;
    std::string path = ImageInterface::BACKEND_SCANS_POINT;
    
    while (all_cameras_done != true) {
        show_progress(input_type, i, frame_count);
        InferenceOutputItem output_item;
        if (!results_queue->pop(output_item)) {
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Something went wrong in post_process while loop");
			system_message_queue->push(msg);
            continue;
        }
        auto& frame_to_draw = output_item.org_frame;
        auto bboxes = parse_nms_data(output_item.output_data_and_infos[0].first, class_count);
         
        bool should_door_open = false;
        
		double max_x = 0.0 , max_y = 0.0;
         // ====== DEBUG CODE START ======
		std::cout << "======== FRAME " << i << " DETECTIONS ========\n";
	
		std::string max_class_name;
		float max = -1.f;
		
		if (bboxes.empty()) {
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "No objects detected in this frame");
			system_message_queue->push(msg);
			std::cout << "No objects detected in this frame.\n";
		} 
		else {
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Object detected");
			system_message_queue->push(msg);
			std::cout << "Detected " << bboxes.size() << " objects:\n";
			
			
			for (size_t j = 0; j < bboxes.size(); ++j) {
				const auto &bbox = bboxes[j];
				std::string class_name = get_coco_name_from_int(static_cast<int>(bbox.class_id));
				float confidence = bbox.bbox.score * 100.f;

				if (confidence > max) { 
					max = confidence; 
					max_class_name = class_name;
					max_x = bbox.bbox.x_max;
					max_y = bbox.bbox.y_max;
				}

				std::cout << j + 1 << ". Class: " << class_name
						  << " (ID: " << bbox.class_id << ")"
						  << " Confidence: " << std::fixed << std::setprecision(2) << confidence << "%"
						  << " Position: [" << bbox.bbox.x_min << ", " << bbox.bbox.y_min << ", "
						  << bbox.bbox.x_max << ", " << bbox.bbox.y_max << "]\n";
			}
			
			scans.push_back(ScanRequestDTO(max_class_name, max, max_y, max_x));
			
			std::cout << "Top-confidence: " << max_class_name << " (" 
					  << std::fixed << std::setprecision(2) << max << "%)\n";
			
			if(scans.size() == num_camera){
					should_door_open = send_to_server(client, host, port, path);
					std::cout << "Should door open: " << should_door_open<< "\n";
					if(should_door_open){
						arduino.setServoAngle(30);
						std::cout << "hayrullah kutuk nere" << std::endl;
						SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Servo Angle is set to 45");
						system_message_queue->push(msg);
					}
					else{
						arduino.setServoAngle(150);
						SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Servo Angle is set to 135");
						system_message_queue->push(msg);
					}
					scans.clear();
			}
		}
		
		
		std::cout << "Sending " << scans.size() << " scans to server..." << std::endl;
    
		/*
		bool success = client.sendScans(host, port, path, scans);
    
		if (success) {
			std::cout << "Scans successfully sent to server" << std::endl;
		} else {
			std::cerr << "Failed to send scans to server" << std::endl;
		}
		
		scans.clear();
		*/

		// ====== DEBUG CODE END ======


		// ========== OBJECT_INFO ==========
		{
			
			auto [base_name, healthy] = parse_class_string(max_class_name);

			object_info info(base_name,                 // sınıf
							 static_cast<double>(max), // güven
							 0,     //x
							 0,    // y
							 healthy);                  // is_healthy

			std::cout << "[object_info] "
					  << info.get_class() << ", conf "
					  << info.get_confidence() << "%, ("
					  << info.get_x() << ',' << info.get_y() << "), healthy="
					  << std::boolalpha << info.get_is_healthy() << '\n';
		}
		// =================================



       
       
       
		/*
        if(class_names.size() == ImageInterface::CAMERA_NUMBER){
            //std::string result = decide_class_of_frames(class_names);
            std::cout << result << std::endl;
            // mekanik_kol(result); // FOR FINAL RESULT, THIS WILL CONTROL MEKANIK_KOL (PROTOTYPE)
        }
        */
        
        
        
        
        // opencv rframe jpg formatında sıkıştırman gerekiyor
        draw_bounding_boxes(frame_to_draw, bboxes);
        
        std::filesystem::create_directories("photos");          // photos yoksa aç
        std::string filename = "photos/deneme_" + std::to_string(i) + ".jpg";
		cv::imwrite(filename, frame_to_draw);
		i++;
    }
    release_resources(capture, video, input_type);
    return HAILO_SUCCESS;
}

void preprocess_video_frames(cv::VideoCapture &capture,
                          uint32_t width, uint32_t height) {
    cv::Mat org_frame;
    while (true) {
        capture >> org_frame;
        if (org_frame.empty()) {
            SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "Something went wrong while take preprocess_video_frames");
			system_message_queue->push(msg);
            preprocessed_queue->stop();
            break;
        }        
        auto preprocessed_frame_item = create_preprocessed_frame_item(org_frame, width, height);
        preprocessed_queue->push(preprocessed_frame_item);
    }
}


//chatgpt white mask function DOĞRU ÇALIŞIYOR SİLME
cv::Mat detectFirstVerticalLinesFromCenter(const cv::Mat &frame,
                                           double slopeTol = 0.5,
                                           int    mergeTol = 15)
{
    if (frame.empty()){ 
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "First Vertical Lines couldn't detect, frame is empty!");
		system_message_queue->push(msg);
		return {};
	}
    
    cv::Mat gray, edges;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 50, 150);

    std::vector<cv::Vec4i> raw;
    cv::HoughLinesP(edges, raw,
                    1, CV_PI / 180,
                    80,          // oy eşiği
                    40, 15);     // minLen, maxGap

    
    struct Group { double xSum; int cnt; };
    std::vector<Group> groups;

    for (const auto &l : raw) {
        int dx = l[2] - l[0];
        int dy = l[3] - l[1];
        if (std::abs(dx) > slopeTol * std::abs(dy)) continue;   // yeterince dikey değil

        double xMid = 0.5 * (l[0] + l[2]);

        bool merged = false;
        for (auto &g : groups) {
            double xAvg = g.xSum / g.cnt;
            if (std::abs(xMid - xAvg) <= mergeTol) {
                g.xSum += xMid; ++g.cnt;
                merged = true;
                break;
            }
        }
        if (!merged) groups.push_back({xMid, 1});
    }

    if (groups.empty()) return frame.clone();

    
    const int cx = frame.cols / 2;
    double leftX  = -1, rightX = -1;
    double leftDist  = std::numeric_limits<double>::max();
    double rightDist = std::numeric_limits<double>::max();

    for (const auto &g : groups) {
        double x = g.xSum / g.cnt;
        if (x < cx) {                         // sol taraf
            double d = cx - x;
            if (d < leftDist) { leftDist = d; leftX = x; }
        } else if (x > cx) {                  // sağ taraf
            double d = x - cx;
            if (d < rightDist) { rightDist = d; rightX = x; }
        }
    }

    
    cv::Mat out = frame.clone();
    int h = frame.rows;

    if (leftX >= 0)
        cv::line(out, {static_cast<int>(std::round(leftX)), 0},
                       {static_cast<int>(std::round(leftX)), h - 1},
                       {0, 0, 255}, 2);

    if (rightX >= 0)
        cv::line(out, {static_cast<int>(std::round(rightX)), 0},
                       {static_cast<int>(std::round(rightX)), h - 1},
                       {0, 0, 255}, 2);

    return out;
}
//end of white mask func



 
std::pair<int,int> firstVerticalLineXsFromCenter(const cv::Mat &frame,
                                                 double slopeTol = 0.5,
                                                 int    mergeTol = 15)
{
    if (frame.empty()){ 
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "First Vertical Lines Xs From Center couldn't detect. Frame is empty, returning {-1, -1}");
		system_message_queue->push(msg);
		return {-1, -1};
	}

    // Kenar ve Hough
    cv::Mat gray, edges;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 50, 150);

    std::vector<cv::Vec4i> raw;
    cv::HoughLinesP(edges, raw, 1, CV_PI / 180, 80, 40, 15);

    // Grupla
    struct G { double xSum; int cnt; };
    std::vector<G> groups;
    for (const auto &l : raw) {
        int dx = l[2] - l[0], dy = l[3] - l[1];
        if (std::abs(dx) > slopeTol * std::abs(dy)) continue;

        double xMid = 0.5 * (l[0] + l[2]);
        bool merged = false;
        for (auto &g : groups) {
            double xAvg = g.xSum / g.cnt;
            if (std::abs(xMid - xAvg) <= mergeTol) {
                g.xSum += xMid; ++g.cnt; merged = true; break;
            }
        }
        if (!merged) groups.push_back({xMid, 1});
    }

    int cx = frame.cols / 2;
    int leftX = -1, rightX = -1;
    double leftDist = std::numeric_limits<double>::max();
    double rightDist = std::numeric_limits<double>::max();

    for (const auto &g : groups) {
        double x = g.xSum / g.cnt;
        if (x < cx && cx - x < leftDist)  { leftDist = cx - x;  leftX  = int(std::round(x)); }
        if (x > cx && x - cx < rightDist) { rightDist = x - cx; rightX = int(std::round(x)); }
    }

    return {leftX, rightX};
}


inline cv::Mat cropBetweenXs(const cv::Mat &src, int leftX, int rightX)
{
    if (leftX < 0 || rightX < 0 || leftX >= rightX){   // çizgi bulunamadıysa kırpma yok
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Couldn't find vertical lines. So, no need to crop that frame");
		system_message_queue->push(msg);
        return src;
	}

    leftX  = std::max(0,                 leftX);
    rightX = std::min(src.cols - 1,      rightX);

    cv::Rect roi(leftX, 0, rightX - leftX + 1, src.rows);
    return src(roi).clone();
}
// ————————————————————————————————————————————————————————————————


inline cv::Vec3d meanCenterRGB(const cv::Mat &frame,
                               int winW = 10, int winH = 10)
{
    CV_Assert(!frame.empty() && frame.channels() == 3);
    CV_Assert(winW > 0 && winH > 0);

    // Pencerenin sol‑üst köşesi
    int x0 = std::clamp(frame.cols / 2 - winW / 2, 0, frame.cols - 1);
    int y0 = std::clamp(frame.rows / 2 - winH / 2, 0, frame.rows - 1);

    // Pencere, görüntü sınırlarını aşmasın
    int w = std::min(winW, frame.cols - x0);
    int h = std::min(winH, frame.rows - y0);

    cv::Scalar m = cv::mean(frame(cv::Rect(x0, y0, w, h)));  // BGR

    return { m[2], m[1], m[0] };  // R, G, B
}



inline cv::Mat whiteOutSameTone(const cv::Mat   &frame,
                                const cv::Vec3d &meanRGB,
                                double luminTolPercent            = 50.0,
                                const cv::Vec3d &colorTolPercentRGB = {5, 5, 5})
{
    CV_Assert(!frame.empty() && frame.channels() == 3);

    // Referanslar (R,G,B)  —  frame pikseli (B,G,R) olduğundan dikkat
    const double meanR = meanRGB[0], meanG = meanRGB[1], meanB = meanRGB[2];
    const double eps   = 1e-6;   // sıfıra bölünme koruması

    // Ön‑hesaplamalar
    const double luminTol = luminTolPercent / 100.0;
    const double tolR = colorTolPercentRGB[0] / 100.0;
    const double tolG = colorTolPercentRGB[1] / 100.0;
    const double tolB = colorTolPercentRGB[2] / 100.0;

    cv::Mat out = frame.clone();

    for (int y = 0; y < frame.rows; ++y) {
        const cv::Vec3b *srcRow = frame.ptr<cv::Vec3b>(y);
        cv::Vec3b       *dstRow = out.ptr<cv::Vec3b>(y);

        for (int x = 0; x < frame.cols; ++x) {
            double b = srcRow[x][0];
            double g = srcRow[x][1];
            double r = srcRow[x][2];

            // Oranlar (Pi / mean_i)
            double rB = b / std::max(meanB, eps);
            double rG = g / std::max(meanG, eps);
            double rR = r / std::max(meanR, eps);

            double rAvg = (rB + rG + rR) / 3.0;

            // Parlaklık ve renk farkı koşulları
            bool okLumin = std::abs(rAvg - 1.0) <= luminTol;
            bool okColor =
                  std::abs(rB - rAvg) <= tolB &&
                  std::abs(rG - rAvg) <= tolG &&
                  std::abs(rR - rAvg) <= tolR;

            if (okLumin && okColor)
                dstRow[x] = cv::Vec3b(255, 255, 255);   // beyaza boya
        }
    }
    return out;
}





bool framesDifferAboveTol(const cv::Mat& f1,               // referans kare
                          const cv::Mat& f2,               // karşılaştırılan kare
                          double diffThresholdPercent,     // % fark eşiği
                          double luminTolPercent   = 60.0, // ton toleransı
                          const cv::Vec3d& colorTolPercentRGB = {5, 5, 5})
{
    if (f1.empty() || f2.empty()) return false;
    if (f1.size() != f2.size() || f1.type() != f2.type()) return false;
    CV_Assert(f1.type() == CV_8UC3);        // 3 kanallı, 8‑bit BGR beklenir

    const double eps = 1e-6;
    const double luminTol = luminTolPercent / 100.0;
    const double tolB = colorTolPercentRGB[2] / 100.0; // BGR sırası → RGB
    const double tolG = colorTolPercentRGB[1] / 100.0;
    const double tolR = colorTolPercentRGB[0] / 100.0;

    std::size_t changed = 0;
    const std::size_t total = static_cast<std::size_t>(f1.total());

    for (int y = 0; y < f1.rows; ++y)
    {
        const cv::Vec3b* p1 = f1.ptr<cv::Vec3b>(y);
        const cv::Vec3b* p2 = f2.ptr<cv::Vec3b>(y);

        for (int x = 0; x < f1.cols; ++x)
        {
            double b1 = p1[x][0], g1 = p1[x][1], r1 = p1[x][2];
            double b2 = p2[x][0], g2 = p2[x][1], r2 = p2[x][2];

            // Kanal oranları (ikinci kare / ilk kare)
            double rB = b2 / std::max(b1, eps);
            double rG = g2 / std::max(g1, eps);
            double rR = r2 / std::max(r1, eps);

            double rAvg = (rB + rG + rR) / 3.0;

            bool okLumin = std::abs(rAvg - 1.0) <= luminTol;
            bool okColor =
                  std::abs(rB - rAvg) <= tolB &&
                  std::abs(rG - rAvg) <= tolG &&
                  std::abs(rR - rAvg) <= tolR;

            if (!(okLumin && okColor))
                ++changed;
        }
    }
    double diffPercent = (static_cast<double>(changed) / total) * 100.0;
    return diffPercent > diffThresholdPercent;
}



cv::Point2d diffCentroidTol(const cv::Mat& f1,
                            const cv::Mat& f2,
                            double diffThresholdPercent,
                            double luminTolPercent          = 10.0,
                            const cv::Vec3d& colorTolPercentRGB = {5, 5, 5})
{
    if (f1.empty() || f2.empty())                 return {-1, -1};
    if (f1.size() != f2.size() || f1.type() != f2.type()) return {-1, -1};
    CV_Assert(f1.type() == CV_8UC3);              // BGR, 8‑bit

    const double eps      = 1e-6;
    const double luminTol = luminTolPercent / 100.0;
    const double tolR     = colorTolPercentRGB[0] / 100.0;
    const double tolG     = colorTolPercentRGB[1] / 100.0;
    const double tolB     = colorTolPercentRGB[2] / 100.0;

    std::size_t changed = 0;
    double sumX = 0.0, sumY = 0.0;

    for (int y = 0; y < f1.rows; ++y) {
        const cv::Vec3b* p1 = f1.ptr<cv::Vec3b>(y);
        const cv::Vec3b* p2 = f2.ptr<cv::Vec3b>(y);

        for (int x = 0; x < f1.cols; ++x) {
            double b1 = p1[x][0], g1 = p1[x][1], r1 = p1[x][2];
            double b2 = p2[x][0], g2 = p2[x][1], r2 = p2[x][2];

            double rB = b2 / std::max(b1, eps);
            double rG = g2 / std::max(g1, eps);
            double rR = r2 / std::max(r1, eps);
            double rAvg = (rB + rG + rR) / 3.0;

            bool okLumin = std::abs(rAvg - 1.0) <= luminTol;
            bool okColor =
                  std::abs(rB - rAvg) <= tolB &&
                  std::abs(rG - rAvg) <= tolG &&
                  std::abs(rR - rAvg) <= tolR;

            if (!(okLumin && okColor)) {
                ++changed;
                sumX += x;
                sumY += y;
            }
        }
    }

    if (changed == 0) return {-1, -1};

    double diffPercent =
        (static_cast<double>(changed) / f1.total()) * 100.0;

    if (diffPercent <= diffThresholdPercent) return {-1, -1};

    return {sumX / changed, sumY / changed};   // kütle merkezi
}


inline bool isCenterBetweenPoints(const cv::Point2d& p1,
                                  const cv::Point2d& p2,
                                  double frameWidth)
{
    if (frameWidth <= 0){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "Frame width <= 0");
		system_message_queue->push(msg);
		return false;                 // geçersiz genişlik
	}
    const double centerX = frameWidth * 0.5;           // görüntü orta noktası (x)
    const double minX    = std::min(p1.x, p2.x);
    const double maxX    = std::max(p1.x, p2.x);
    if(minX == -1 || maxX == -1)
		return false;

    return centerX >= minX && centerX <= maxX;
}






void grabLoop(int camId, CamBuf &buf, std::atomic<bool> &run,
              uint32_t width, uint32_t height, UdpSender udp)
{
    cv::VideoCapture cap(camId, cv::CAP_V4L2);
    
    
    if (!cap.isOpened()) {
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "At least one of 3 cameras couldn't opened!");
		system_message_queue->push(msg);
		
        std::cerr << "Kamera " << buf.camId << " açılmadı!\n";
        // run = false;
        if (buf.camId == 0) camera0_active = false;
		else if (buf.camId == 1) camera1_active = false;
		else if (buf.camId == 2) camera2_active = false;
		
		--active_cameras;
		num_camera--;
		
		pthread_barrier_wait(&sync_barrier);
		
        return;
    }
    
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS,          30);

    // — 1) Arka‑plan karesi + dikey çizgi koordinatları —
    cv::Mat background;
    cap.read(background);                       // ilk kareyi çek
    auto [leftX, rightX] = firstVerticalLineXsFromCenter(background);
    if (leftX == -1 && rightX == -1){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "There isn't any vertical lines. So, set leftX = 0, rightX = 640");
		system_message_queue->push(msg);
		leftX = 0;
		rightX = 640;
	}
    std::cout << "Left X: "  << leftX << ", Right X: " << rightX << '\n';
              
    cv::Mat cropped_background = cropBetweenXs(background, leftX, rightX);
    cv::Mat Test;
    
    //Object deneme 1
    cv::Vec3d mean_rgb = meanCenterRGB(cropped_background);
    cv::Mat mean_bg_frame;
    cv::Mat mean_frame;
    
        // background frame whiteout
            if(buf.camId == 0 && camera0_active == true){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
            }
            else if(buf.camId == 1 && camera1_active == true){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
            }
            
            else if(buf.camId == 2 && camera2_active == true){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
            }
            
            std::string filePath;
			
        cv::Point2d difference;
        cv::Mat start_frame = mean_bg_frame.clone();
        cv::Mat tmp;
        
        cap.read(tmp);
        
        cv::Mat tmp2 = cropBetweenXs(tmp, leftX, rightX);
        
        cv::Mat previous_frame = mean_bg_frame.clone();
       
		cv::Point2d previous_difference ;
        
			if(buf.camId == 0 && camera0_active == true){
                previous_difference = diffCentroidTol(previous_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_0);
            }
            else if(buf.camId == 1 && camera1_active == true){
                previous_difference = diffCentroidTol(previous_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_1);
            }
            
            else if(buf.camId == 2 && camera2_active == true){
                previous_difference = diffCentroidTol(previous_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_2);
            }
        
         //previous_difference = diffCentroidTol(previous_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE);
        
    // — 2) Sürekli okuma, kırpma ve paylaşılan arabellek —
    
    while (run) {
        cv::Mat frame;
        // frame is a cv::Mat that already contains your BGR pixels

        if (!cap.read(frame) || frame.empty()){ 
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Frame is empty! (grabLoop)");
			system_message_queue->push(msg);
			break; 
		}
        
        // size_t rawBytes = frame.total() * frame.elemSize();   // rows*cols*channels
		
		// std::cout << "size" << rawBytes << std::endl;
        
        Test = detectFirstVerticalLinesFromCenter(background); // çizgileri görmek için kullanıyoruz
        cv::Mat cropped = cropBetweenXs(frame, leftX, rightX);


            if(buf.camId == 0 && camera0_active == true){
                mean_frame = whiteOutSameTone(cropped, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            else if(buf.camId == 1 && camera1_active == true){
                mean_frame = whiteOutSameTone(cropped, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            
            else if(buf.camId == 2 && camera2_active == true){
                mean_frame = whiteOutSameTone(frame, mean_rgb, ImageInterface::LUMIN_TOL_PERCENT, ImageInterface::COLOR_TOL_PERCENT_RGB);
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            
            /*
            TO GATHER EMPHTY İMAGES
            
            if(count < ImageInterface::SAVE_NUMBER + 500){
				filePath = std::string("preprocess_photo/") + "image" + std::to_string(count) + ".jpg";
				cv::imwrite(filePath, mean_frame);
				count++;
			}
			else{
				std::cout << "FOTOGRAF CEKME BİTTİ" << std::endl;
			}
			* */
			
			if(buf.camId == 0 && camera0_active == true){
                difference =  diffCentroidTol(mean_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_0);
            }
            else if(buf.camId == 1 && camera1_active == true){
                difference =  diffCentroidTol(mean_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_1);
            }
            
            else if(buf.camId == 2 && camera2_active == true){
                difference =  diffCentroidTol(mean_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE_2);
            }
                    
            
            
          
  
            //difference =  diffCentroidTol(mean_frame, previous_frame, ImageInterface::THRESHOLD_DIFFERENCE);
            
            

            
            
            if (isCenterBetweenPoints(difference, previous_difference,rightX -leftX)){
                // std::cout << "nesne ortada algılandı " << buf.camId << std::endl;
                // ----- COOL-DOWN KONTROLÜ -----
                auto  now          = std::chrono::steady_clock::now();
                bool  can_capture  = true;

                if (last_capture_ts.count(camId) &&
                    now - last_capture_ts[camId] < CAPTURE_COOLDOWN)
                {
                    can_capture = false;          // hâlâ “soğuma” süresindeyiz
                }

                if (can_capture) {
                    last_capture_ts[camId] = now; // yeni zaman damgası

                    /* — buraya ESAS tetikleme işleminiz — */
                    buf.object_detection = true;          // kuyruğa itmek için bayrak
                    
                    
                    
                    std::cout << difference << std::endl;
					std::cout << previous_difference << std::endl;
                    std::cout << (rightX - leftX) / 2 << std::endl;
                    
                    /*
                    
                    if(count <= ImageInterface::SAVE_NUMBER + 500){
					
					std::cout << "Resim alindi " << std::endl;
					
                    filePath = std::string("photos/") + "image" + std::to_string(count) + ".jpg";
					cv::imwrite(filePath, mean_frame);
					count++;
					
					}
					*/
					
					
					
					
                    // veya doğrudan:
                    // cv::imwrite("images/frame.jpg", cur);
                }

            }
            else{
                buf.object_detection = false;
            }
            
            
            
            
            previous_difference = difference;
            
			/*
            std::cout << difference << std::endl;
            
            std::cout << previous_difference << std::endl;
            
            */
            previous_frame = mean_frame.clone();
            
            
            
            if(buf.camId == 0 && camera0_active == true){
				std::lock_guard<std::mutex> lk(buf.m);
				 // buf.frame = mean_frame.clone(); 
				 buf.frame = std::move(cropped);

				
				udp.send(buf.camId, frame);
			}
            else if(buf.camId == 1 && camera1_active == true){
				std::lock_guard<std::mutex> lk(buf.m);
				// buf.frame = mean_frame.clone(); 
				 buf.frame = std::move(cropped);

				
				udp.send(buf.camId, frame);
			}
            
            else if(buf.camId == 2 && camera2_active == true) {
				std::lock_guard<std::mutex> lk(buf.m);
				
				buf.frame = std::move(cropped);
				// buf.frame = mean_frame.clone();
				
				udp.send(buf.camId, frame);
			}
        
    }
    
    

    if (--active_cameras == 0){
        SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "number of active cameras is 0, cameras will be closed.");
		system_message_queue->push(msg);
        all_cameras_done = true;
		std::cout << "camera düştü" << std::endl;
	}
	
	pthread_barrier_wait(&sync_barrier);
	return;
}




void camera_image_frames(const std::string &input_path,
                          uint32_t width, uint32_t height) {
    cv::Mat org_frame = cv::imread(input_path);
    auto preprocessed_frame_item = create_preprocessed_frame_item(org_frame, width, height);
    preprocessed_queue->push(preprocessed_frame_item);
}


void preprocess_image_frames(const std::string &input_path,
                          uint32_t width, uint32_t height) {
    cv::Mat org_frame = cv::imread(input_path);
    auto preprocessed_frame_item = create_preprocessed_frame_item(org_frame, width, height);
    preprocessed_queue->push(preprocessed_frame_item);
}
void preprocess_directory_of_images(const std::string &input_path,
                                uint32_t width, uint32_t height) {
    for (const auto &entry : fs::directory_iterator(input_path)) {
            preprocess_image_frames(entry.path().string(), width, height);
    }
}

hailo_status run_preprocess(CommandLineArgs args, AsyncModelInfer &model, 
                            InputType &input_type, cv::VideoCapture &capture) {

    auto model_input_shape = model.get_infer_model()->hef().get_input_vstream_infos().release()[0].shape;
    uint32_t target_height = model_input_shape.height;
    uint32_t target_width = model_input_shape.width;
    print_net_banner(get_hef_name(args.detection_hef), std::ref(model.get_inputs()), std::ref(model.get_outputs()));


    std::atomic<bool> running(true);
    
    CamBuf buffer0, buffer1, buffer2;
    
    buffer0.camId = 0;
    buffer1.camId = 1;
    buffer2.camId = 2;
    
	 
	 pthread_barrier_init(&sync_barrier, nullptr, 3);
	
	std::thread t0;
	std::thread t1;
	std::thread t2;
	
	UdpSender udp0(ImageInterface::DESKTOP_IP_UDP,ImageInterface::UDP_COMMS_PORT); 
	
		try {
			t0 = std::thread(grabLoop, 0,
							 std::ref(buffer0),
							 std::ref(running),
							 target_width, target_height,
							 std::cref(udp0));
		}
		catch (const std::system_error& e) {    // creation failed
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Thread 0 couldn't be started");
			system_message_queue->push(msg);
			std::cerr << "Could not start thread: " << e.what() << '\n';
		}

		if (!t0.joinable()) {                   // safety check
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "Thread 0 is not running!");
			system_message_queue->push(msg);
			std::cerr << "Thread not running!\n";
		}
		
		/*
		 * not controlling the thread operation part
	 std::thread t0(grabLoop, 0, std::ref(buffer0), std::ref(running),
				target_width, target_height, std::cref(udp0));  
		*/
		
	
	
	
	UdpSender udp1(ImageInterface::DESKTOP_IP_UDP, ImageInterface::UDP_COMMS_PORT);
	
		try {
			t1 = std::thread(grabLoop, 2,
							 std::ref(buffer1),
							 std::ref(running),
							 target_width, target_height,
							 std::cref(udp1));
		}
		catch (const std::system_error& e) {    // creation failed
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Thread 1 couldn't be started");
			system_message_queue->push(msg);
			std::cerr << "Could not start thread: " << e.what() << '\n';
		}

		if (!t1.joinable()) {                   // safety check
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "Thread 1 is not running!");
			system_message_queue->push(msg);
			std::cerr << "Thread not running!\n";
		}
	
	
	
	/*
    std::thread t1(grabLoop, 2, std::ref(buffer1), std::ref(running),
				target_width, target_height, std::cref(udp1));
	* */
				
				
				    
    
	UdpSender udp2(ImageInterface::DESKTOP_IP_UDP, ImageInterface::UDP_COMMS_PORT);  
	
		try {
			t2 = std::thread(grabLoop, 4,
							 std::ref(buffer2),
							 std::ref(running),
							 target_width, target_height,
							 std::cref(udp2));
		}
		catch (const std::system_error& e) {    // creation failed
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Thread 2 couldn't be started");
			system_message_queue->push(msg);
			std::cerr << "Could not start thread: " << e.what() << '\n';
		}

		if (!t2.joinable()) {                   // safety check
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::WARNING, "Thread 2 is not running!");
			system_message_queue->push(msg);
			std::cerr << "Thread not running!\n";
		}
	
	/* 
    std::thread t2(grabLoop, 4, std::ref(buffer2), std::ref(running),
				target_width, target_height, std::cref(udp2));  
	*/
    
    
    if(camera0_active == true){
		cv::namedWindow("Cam0", cv::WINDOW_NORMAL);
		cv::moveWindow("Cam0", 50, 50);
	}
    
    if(camera1_active == true){
		cv::namedWindow("Cam1", cv::WINDOW_NORMAL);
		cv::moveWindow("Cam1", 420, 50);
	}
    
    if(camera2_active == true){
		cv::namedWindow("Cam2", cv::WINDOW_NORMAL);
		cv::moveWindow("Cam2", 640, 50);
    }
    
    
    while (running) {
        
        char c = static_cast<char>(cv::waitKey(1));
        
        if(camera0_active == true)
        {
            std::lock_guard<std::mutex> lk(buffer0.m);
            if (!buffer0.frame.empty())
                cv::imshow("Cam0", buffer0.frame);
            
            if (system_ready.load() && buffer0.object_detection == true) { // seko system_ready.load() attı başlangıç delayı için
				auto preprocessed_frame_item = create_preprocessed_frame_item(buffer0.frame, target_width, target_height);
				preprocessed_queue->push(preprocessed_frame_item);
				std::cout << "Frame alındı ve queue'ya eklendi.0" << std::endl;
				buffer0.object_detection = false;
            } 
            
        }
        if(camera1_active == true)
        {
            std::lock_guard<std::mutex> lk(buffer1.m);
            if (!buffer1.frame.empty())
                cv::imshow("Cam1", buffer1.frame);
            
            if (system_ready.load() && buffer1.object_detection == true){ // seko system_ready.load() attı başlangıç delayı için
				auto preprocessed_frame_item = create_preprocessed_frame_item(buffer1.frame, target_width, target_height);
				preprocessed_queue->push(preprocessed_frame_item);
				std::cout << "Frame alındı ve queue'ya eklendi.1" << std::endl;
				buffer1.object_detection = false;
            } 
            
        }
        if(camera2_active == true)
        {
            std::lock_guard<std::mutex> lk(buffer2.m);
            if (!buffer2.frame.empty())
                cv::imshow("Cam2", buffer2.frame);
            
            if (system_ready.load() && buffer2.object_detection == true){ // seko system_ready.load() attı başlangıç delayı için
				auto preprocessed_frame_item = create_preprocessed_frame_item(buffer2.frame, target_width, target_height);
				preprocessed_queue->push(preprocessed_frame_item);
				std::cout << "Frame alındı ve queue'ya eklendi.2" << std::endl;
				buffer2.object_detection = false;
            } 
            
        }
        

        if (c == 27){
			SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "ESC is clicked");
			system_message_queue->push(msg);
            running = false;
        }
    }

    t0.join();
    t1.join();
    t2.join();
    
    pthread_barrier_destroy(&sync_barrier);
    
    cv::destroyAllWindows();
    preprocessed_queue->stop(); // queue'yu durdur
    results_queue->stop(); // sonuç kuyruğunu da durdur
    
    
    // camera_continous_frames(target_width,target_height);


    return HAILO_SUCCESS;
}

hailo_status run_inference_async(AsyncModelInfer& model,
                            std::chrono::duration<double>& inference_time) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    while (all_cameras_done != true) {
        PreprocessedFrameItem item;
        if (!preprocessed_queue->pop(item)) {
            continue;
        }
        model.infer(std::make_shared<cv::Mat>(item.resized_for_infer), item.org_frame);
    }
    model.get_queue()->stop();
    auto end_time = std::chrono::high_resolution_clock::now();

    inference_time = end_time - start_time;

    return HAILO_SUCCESS;
}

// mert istek fonksiyon baslangic

// FLASH ARDUINO
bool UploadArduino()
{
	std::string command = "~/bin/arduino-cli compile --fqbn arduino:avr:uno " + InoFilePath + 
                         " && ~/bin/arduino-cli upload -p " + ARDUINO_PORT + " --fqbn arduino:avr:uno " + InoFilePath;
    
    int result = system(command.c_str());
    return (result == 0);
}

// mert istek fonksiyon bitis


int main(int argc, char** argv)
{
	if(!UploadArduino())
	{
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to flash arduino");
		system_message_queue->push(msg);
		return 0;
	}
	
	ArduinoSerial arduino(ImageInterface::ARDUINO_PORT);
	if (!arduino.isConnected())
	{
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to connect to Arduino on");
		system_message_queue->push(msg);
		std::cerr << "Failed to connect to Arduino on " << ImageInterface::ARDUINO_PORT << std::endl;
		// return 1;
	}
	
	HttpServerHandler serverHandler(&arduino);
	serverHandler.Init();
	
	if (!serverHandler.Bind())
	{
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::ERROR, "Failed to bind server to port 8080");
		system_message_queue->push(msg);
		std::cerr << "Failed to bind server to port 8080\n";
		return 1;
	}
	
	std::thread serverThread([&serverHandler]()
							 { serverHandler.Start(); });
	
    size_t class_count = 6; // 80 classes in COCO dataset
    double fps = 30;
    
    std::chrono::duration<double> inference_time;
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::high_resolution_clock::now();
    double org_height, org_width;
    cv::VideoCapture capture;
    size_t frame_count;
    InputType input_type;
    
     std::thread logger_thread(log_system_stats);
     std::thread message_thread(log_system_messages);

    CommandLineArgs args = parse_command_line_arguments(argc, argv);
    AsyncModelInfer model(args.detection_hef, results_queue);
    input_type = determine_input_type(args.input_path, std::ref(capture), org_height, org_width, frame_count);

    auto preprocess_thread = std::async(run_preprocess,
                                        args,
                                        std::ref(model),
                                        std::ref(input_type),
                                        std::ref(capture));



    auto inference_thread = std::async(run_inference_async,
                                    std::ref(model),
                                    std::ref(inference_time));

    auto output_parser_thread = std::async(run_post_process,
                                std::ref(input_type),
                                args,
                                org_height,
                                org_width,
                                frame_count,
                                std::ref(capture),
                                std::ref(arduino),
                                class_count,
                                fps);
                                
	// Seko delay baslangic
	std::this_thread::sleep_for(std::chrono::seconds(ImageInterface::STARTUP_DELAY_SECONDS));
	system_ready = true;   
	// Seko delay sonu
	
	
	
	
    hailo_status status = wait_and_check_threads(
        preprocess_thread,    "Preprocess",
        inference_thread,     "Inference",
        output_parser_thread, "Postprocess "
    );
    
    keep_logging = false;
    if (logger_thread.joinable()){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Logger_thread is joinable. logger_thread.join() is called");
		system_message_queue->push(msg); 
		logger_thread.join();
	}
    
    if (message_thread.joinable()){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Message_thread is joinable. message_thread.join() is called");
		system_message_queue->push(msg); 
		message_thread.join();
	}
    
    std::cout << "Stopping server...\n";
	serverHandler.Stop();

	if (serverThread.joinable()){
		SystemLogMessageDTO msg = SystemLogMessageDTO(SystemLogMessageDTO::LogLevel::INFO, "Server_thread is joinable. server_thread.join() is called");
		system_message_queue->push(msg); 
		serverThread.join();
	}

	std::cout << "System shut down gracefully." << std::endl;
    
    
    // Serial communication loop here
    
    if (HAILO_SUCCESS != status) {
        return status;
    }

    if(!input_type.is_camera) {
        std::chrono::time_point<std::chrono::system_clock> t_end = std::chrono::high_resolution_clock::now();
        print_inference_statistics(inference_time, args.detection_hef, frame_count, t_end - t_start);
    }

    return HAILO_SUCCESS;
}




