

#include "async_inference.hpp"
#include "utils.hpp"
#include <thread>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>

#define CAMERAS 2

std::atomic_int active_cameras(CAMERAS); 

std::atomic_bool all_cameras_done(false);



typedef struct CamBuf{
    int camId;
    cv::Mat frame;
    std::mutex m;
    bool object_detection = false;
    std::atomic_bool camera = false;
}CamBuf;

// seko eklemeler

// Her kamera için MOG2 arka plan çıkarıcılarını saklamak için
static std::unordered_map<int, cv::Ptr<cv::BackgroundSubtractorMOG2>> background_subtractors;
// Her kamera için son tetikleme zamanını saklamak için (debounce için)
static std::unordered_map<int, std::chrono::steady_clock::time_point> last_trigger_times;
// Her kamera için fotoğraf sayacını saklamak için
static std::unordered_map<int, int> photo_counters;

// seko eklemeler bitiş



/////////// Constants ///////////
constexpr size_t MAX_QUEUE_SIZE = 60;
/////////////////////////////////

std::shared_ptr<BoundedTSQueue<PreprocessedFrameItem>> preprocessed_queue =
    std::make_shared<BoundedTSQueue<PreprocessedFrameItem>>(MAX_QUEUE_SIZE);

std::shared_ptr<BoundedTSQueue<InferenceOutputItem>>   results_queue =
    std::make_shared<BoundedTSQueue<InferenceOutputItem>>(MAX_QUEUE_SIZE);

void release_resources(cv::VideoCapture &capture, cv::VideoWriter &video, InputType &input_type) {
    if (input_type.is_video) {
        video.release();
    }
    if (input_type.is_camera) {
        capture.release();
        cv::destroyAllWindows();
    }
    preprocessed_queue->stop();
    results_queue->stop();
}

hailo_status run_post_process(
    InputType &input_type,
    CommandLineArgs args,
    int org_height,
    int org_width,
    size_t frame_count,
    cv::VideoCapture &capture,
    size_t class_count = 80,
    double fps = 30) 
    {

    cv::VideoWriter video;
    if (input_type.is_video || (input_type.is_camera && args.save)) {    
        init_video_writer("./processed_video.mp4", video, fps, org_width, org_height);
    }
    int i = 0;
    while (all_cameras_done != true) {
        
        show_progress(input_type, i, frame_count);
        InferenceOutputItem output_item;
        if (!results_queue->pop(output_item)) {
            continue;
        }
        auto& frame_to_draw = output_item.org_frame;
        auto bboxes = parse_nms_data(output_item.output_data_and_infos[0].first, class_count);
         
        

         // ====== DEBUG CODE START ======
        
        std::cout << "======== FRAME " << i << " DETECTIONS ========" << std::endl;
        if (bboxes.empty()) {
            std::cout << "No objects detected in this frame." << std::endl;
        } else {
            std::cout << "Detected " << bboxes.size() << " objects:" << std::endl;
            for (size_t j = 0; j < bboxes.size(); j++) {
                const auto& bbox = bboxes[j];
                std::string class_name = get_coco_name_from_int(static_cast<int>(bbox.class_id));
                float confidence = bbox.bbox.score * 100.0f;
                
                std::cout << j+1 << ". Class: " << class_name 
                          << " (ID: " << bbox.class_id << ")"
                          << " Confidence: " << std::fixed << std::setprecision(2) << confidence << "%"
                          << " Position: [" 
                          << bbox.bbox.x_min << ", " << bbox.bbox.y_min << ", " 
                          << bbox.bbox.x_max << ", " << bbox.bbox.y_max << "]" 
                          << std::endl;
            }
        }
        std::cout << "=======================================" << std::endl;
        // ====== DEBUG CODE END ======
        draw_bounding_boxes(frame_to_draw, bboxes);
        
        /*
        if (input_type.is_video || (input_type.is_camera && args.save)) {
            video.write(frame_to_draw);
        }
        
        if (!show_frame(input_type, frame_to_draw)) {
            break; // break the loop if input is from camera and user pressed 'q' 
        }     
        else if (input_type.is_image || input_type.is_directory) {
            cv::imwrite("processed_image_" + std::to_string(i) + ".jpg", frame_to_draw);
            if (input_type.is_image) {break;}
            else if (input_type.directory_entry_count - 1 == i) {break;}
        }
        * */
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
            preprocessed_queue->stop();
            break;
        }        
        auto preprocessed_frame_item = create_preprocessed_frame_item(org_frame, width, height);
        preprocessed_queue->push(preprocessed_frame_item);
    }
}



/*
// seko kod
// detect_moving_object_and_trigger fonksiyonunu aşağıdaki gibi güncelleyelim:

// static std::unordered_map'ler fonksiyon dışında veya global/class üyesi olabilir,
// ancak minimal değişiklik için fonksiyon içinde static bırakıyorum.
// static std::unordered_map<int, cv::Ptr<cv::BackgroundSubtractorMOG2>> background_subtractors;
// static std::unordered_map<int, std::chrono::steady_clock::time_point> last_trigger_times;
// static std::unordered_map<int, int> photo_counters;
// Bu static map'ler zaten bir önceki cevabımda tanımlıydı, tekrar yazmıyorum.

void detect_moving_object_and_trigger(int camId, CamBuf &buf, cv::Mat &current_raw_frame) {
    if (background_subtractors.find(camId) == background_subtractors.end()) {
        int history = 500;
        double varThreshold = 50.0; 
        bool detectShadows = false;
        background_subtractors[camId] = cv::createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
        photo_counters[camId] = 0;
        std::cout << "[Cam " << camId << "] MOG2 initialized with varThreshold=" << varThreshold << std::endl;
    }
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = background_subtractors[camId];

    cv::Mat fgMask, processedMask;

    // 1. Arka Plan Çıkarımını Uygula
    pMOG2->apply(current_raw_frame, fgMask);
    // fgMask şimdi (detectShadows=false ile) 0 veya 255 olmalı.
    // Ekstra threshold adımını (şimdilik) kaldıralım, sizin eski kodunuzda yoktu.
    // Eğer fgMask'ta ara değerler görüyorsanız ( unlikely with detectShadows=false ),
    // o zaman cv::threshold(fgMask, fgMask, 128, 255, cv::THRESH_BINARY); eklenebilir.
    // Şimdilik doğrudan fgMask kullanalım.
    processedMask = fgMask.clone();

    cv::morphologyEx(processedMask, processedMask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    // 2. Morfolojik İşlemler (Sizin eski kodunuzdaki sıraya benzer şekilde)
    //cv::erode(processedMask, processedMask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    //cv::dilate(processedMask, processedMask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::morphologyEx(processedMask, processedMask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7)));


    // 3. Konturları Bul
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(processedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // (Geri kalan kısım – merkez kontrolü, debounce, fotoğraf çekme – aynı kalabilir)
    // ... (Bir önceki cevabımdaki 4. adımdan itibaren olan kısım) ...
    // Sadece alan filtresi sizin kodunuzdaki gibi `area > min_contour_area` olmalı,
    // benim kodumda `area < min_contour_area` ile `continue` vardı, bu zaten aynı mantık.
    // `min_contour_area` değerini 1500 olarak belirlemiştik.

    bool object_in_center_this_trigger = false; // Bu satır döngüden önce olmalı

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < 2500.0) { // Sizin eski kodunuzdaki mantıkla aynı (min_contour_area = 1500.0)
            continue;
        }

        cv::Rect bbox = cv::boundingRect(contour);
        cv::rectangle(current_raw_frame, bbox, cv::Scalar(0, 255, 0), 2);

        int bbox_center_x = bbox.x + bbox.width / 2;
        int frame_width = current_raw_frame.cols;
        int center_region_start_x = static_cast<int>(frame_width * 0.40);
        int center_region_end_x   = static_cast<int>(frame_width * 0.60);

        if (bbox_center_x >= center_region_start_x && bbox_center_x <= center_region_end_x) {
            object_in_center_this_trigger = true;

            constexpr int debounce_duration_ms = 2000;
            auto now = std::chrono::steady_clock::now();
            bool can_trigger_action = true;

            if (last_trigger_times.count(camId)) {
                auto last_time = last_trigger_times[camId];
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() < debounce_duration_ms) {
                    can_trigger_action = false;
                }
            }

            if (can_trigger_action) {
                last_trigger_times[camId] = now;
                std::string photo_filename = "images/center_snapshot_cam" + std::to_string(camId) +
                                             "_event" + std::to_string(photo_counters[camId]++) + ".png";
                bool photo_saved = cv::imwrite(photo_filename, current_raw_frame);
                buf.object_detection = true;

                if (photo_saved) {
                    std::cout << "[Cam " << camId << "] Object in center. Photo: " << photo_filename
                              << ". Signaled for AI processing." << std::endl;
                } else {
                    std::cout << "[Cam " << camId << "] Object in center. Photo FAILED to save. Signaled for AI processing." << std::endl;
                }
                break;
            }
        }
    }
}
// seko kod bitiş
*/




// seko grab loop
// grabLoop fonksiyonunuzu bulun ve aşağıdaki gibi güncelleyin:
/*
void grabLoop(int camId, CamBuf &buf, std::atomic<bool> &run,
              uint32_t width, uint32_t height) // width ve height parametreleri zaten vardı, kullanacağız.
{
    cv::VideoCapture cap(camId, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Kamera " << camId << " açılmadı!\n";
        // run = false; // Bu satır tüm sistemi durdurabilir, sadece bu thread için problemse
                       // active_cameras'ı azaltıp doğrudan return daha iyi olabilir.
                       // Orijinal kodda run=false yoktu, sadece cerr vardı. Orijinalini koruyalım.
        active_cameras--; // Bu thread düzgün başlamadığı için aktif kamera sayısını azalt
        if(active_cameras == 0) {
            all_cameras_done = true;
        }
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  width);   // Orijinal kodunuzda target_width/height kullanılıyor,
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);  // bu parametreleri fonksiyona geçirdiğimizden emin olmalıyız.
                                                // Zaten grabLoop imzası width/height alıyor.
    cap.set(cv::CAP_PROP_FPS,          30);

    // Orijinal detect_center_object için kullanılan başlangıçtaki background_frame:
    // cv::Mat background_frame;
    // if (!cap.read(background_frame) || background_frame.empty()){
    //     // std::cout << "background is taken succesfully" << std::endl; // Orijinalde bu mesaj yoktu.
    //     // Eğer background_frame okunamadıysa bir sorun olabilir, MOG2 yine de çalışır ama ilk kareler için
    //     // referansı olmaz. Bu satırları MOG2 kullandığımız için kaldırabiliriz veya yorum yapabiliriz.
    // }

    while (run) {
        cv::Mat tmp;
        if (!cap.read(tmp) || tmp.empty()) {
            std::cout << "[Cam " << camId << "] Kare okunamadı veya video sonu." << std::endl;
            break; // Döngüden çık
        }

        // ESKİ ÇAĞRI: detect_center_object(camId, buf, background_frame, tmp);
        // YENİ ÇAĞRI:
        // Fonksiyonumuz current_raw_frame'i (yani tmp'yi) doğrudan değiştirecek (bounding box çizimi için)
        // ve buf.object_detection bayrağını ayarlayacak.
        detect_moving_object_and_trigger(camId, buf, tmp); // width ve height gerekiyordu, düzeltelim.
                                                           // İmza frame_width, frame_height almıyor, tmp.cols ve tmp.rows kullanabiliriz.
                                                           // Ama tmp'yi fonksiyona yolluyoruz, içinde kullanabilir.


        std::lock_guard<std::mutex> lk(buf.m);
        buf.frame = std::move(tmp); // tmp (muhtemelen üzerine çizim yapılmış haliyle) buf.frame'e taşınır.
    }

    active_cameras--;
    if(active_cameras == 0) {
        all_cameras_done = true;
    }
    std::cout << "[Cam " << camId << "] GrabLoop sonlandı." << std::endl;
}
* */


//chatgpt white mask function DOĞRU ÇALIŞIYOR SİLME
cv::Mat detectFirstVerticalLinesFromCenter(const cv::Mat &frame,
                                           double slopeTol = 0.5,
                                           int    mergeTol = 15)
{
    if (frame.empty()) return {};

    
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



/**
 * Merkezden itibaren ilk sol ve ilk sağ “yaklaşık” dikey çizginin
 * x‑koordinatlarını döndürür.
 *
 * Dönüş: { leftX, rightX }  (bulunamazsa -1)
 */
 
std::pair<int,int> firstVerticalLineXsFromCenter(const cv::Mat &frame,
                                                 double slopeTol = 0.5,
                                                 int    mergeTol = 15)
{
    if (frame.empty()) return {-1, -1};

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
    if (leftX < 0 || rightX < 0 || leftX >= rightX)   // çizgi bulunamadıysa
        return src;                                   // kırpma yok

    leftX  = std::max(0,                 leftX);
    rightX = std::min(src.cols - 1,      rightX);

    cv::Rect roi(leftX, 0, rightX - leftX + 1, src.rows);
    return src(roi).clone();
}
// ————————————————————————————————————————————————————————————————

// Object detection fikir 1

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



/**
 * Ortalama R‑G‑B değerini referans alıp, her kanalda ±(yüzde) tolerans
 * içinde kalan pikselleri mutlak beyaz (255,255,255) yapar.
 *
 *  frame        : BGR görüntü (değişmez, kopya oluşturulur)
 *  meanRGB      : {R, G, B} ortalama (double, 0‑255 arası)
 *  percentRGB   : {pR, pG, pB} → her kanal için ±yüzde toleransı (0‑100)
 *
 *  Dönüş        : yeni cv::Mat (tolerans dâhilinde beyazlanmış)
 *
 *  Örnek:
 *      auto mean  = meanCenterRGB(frame, 20, 20);        // R,G,B
 *      cv::Mat out = whiteOutNearMean(frame, mean,
 *                                     {10, 10, 10});     // ±%10 tolerans
 */
/**
 * Aynı “renk tonu”na (RGB oranları benzer) ve benzer parlaklığa sahip
 * pikselleri mutlak beyaz (255, 255, 255) yapar.
 *
 *  frame              : BGR görüntü
 *  meanRGB            : {R, G, B} referans ortalama (double, 0‑255)
 *  luminTolPercent    : ± parlaklık toleransı  (tek skaler, %)
 *  colorTolPercentRGB : {pR, pG, pB}  →  kanal başına oran farkı toleransı  (%)
 *
 * Mantık
 * ──────
 *   1) piksel oranları  ri = Pi / mean_i   (i = R,G,B)
 *   2) oranların ortalaması   r̄ = (rR+rG+rB)/3
 *   3) r̄  →  parlaklık (luminance) ölçütü
 *      |r̄‑1| ≤ luminTol
 *   4) Her kanal için   |ri‑r̄| ≤ colorTol_i
 *
 * Bu koşulları sağlayan pikseller beyaza boyanır.
 */
 

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





// obj detect FİKİR 1 BİTİŞ



void detect_center_object(int camId, CamBuf &buf, const cv::Mat& bg,
                          const cv::Mat &cur, double centreFrac = 0.5){
                                
        if (bg.empty() || cur.empty()) return;

    cv::Mat diff, gray, mask;
    cv::absdiff(bg, cur, diff);
    cv::cvtColor(diff, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, mask, 100, 255, cv::THRESH_BINARY);

    // quick noise clean‑up (optional)
    cv::erode(mask,  mask, {}, {-1,-1}, 1);
    cv::dilate(mask, mask, {}, {-1,-1}, 1);

    cv::Moments m = cv::moments(mask, true);        // true → use binary image
    if (m.m00 == 0) return;                         // no motion

    int cx = static_cast<int>(m.m10 / m.m00);       // centroid
    int cy = static_cast<int>(m.m01 / m.m00);

    int w  = cur.cols;
    int h  = cur.rows;
    int cw = static_cast<int>(w * centreFrac * 0.5); // half‑width of centre box
    int ch = static_cast<int>(h * centreFrac * 0.5);

    int centreX = w / 2;
    int centreY = h / 2;
    
    
    if (std::abs(cx - centreX) <= cw &&
        std::abs(cy - centreY) <= ch)
    {
        if (buf.camera == false){
            std::cout << "serkan efe" << std::endl;
            buf.object_detection = true;
            buf.camera = true;
            std::cout << "[Cam " << camId << "] motion near centre at ("
                  << cx << "," << cy << ")\n";
        
            cv::imwrite("images/frame.jpg", cur);
        }
    }
    else{
        buf.camera = false;
        std::cout << "ziya bababababab" << std::endl;
        }
    
                              
                              
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
    if (frameWidth <= 0) return false;                 // geçersiz genişlik

    const double centerX = frameWidth * 0.5;           // görüntü orta noktası (x)
    const double minX    = std::min(p1.x, p2.x);
    const double maxX    = std::max(p1.x, p2.x);

    return centerX >= minX && centerX <= maxX;
}






void grabLoop(int camId, CamBuf &buf, std::atomic<bool> &run,
              uint32_t width, uint32_t height)
{
    cv::VideoCapture cap(camId, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Kamera " << camId << " açılmadı!\n";
        run = false;
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS,          30);

    // — 1) Arka‑plan karesi + dikey çizgi koordinatları —
    cv::Mat background;
    cap.read(background);                       // ilk kareyi çek
    auto [leftX, rightX] = firstVerticalLineXsFromCenter(background);
    std::cout << "Left X: "  << leftX
              << ", Right X: " << rightX << '\n';
              
    cv::Mat cropped_background = cropBetweenXs(background, leftX, rightX);
    cv::Mat Test;
    
    //Object deneme 1
    cv::Vec3d mean_rgb = meanCenterRGB(cropped_background);
    cv::Mat mean_bg_frame;
    cv::Mat mean_frame;
    
        // background frame whiteout
            if(buf.camId == 0){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, 80.0, {10,10,10});
            }
            else if(buf.camId == 1){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, 80.0, {10,10,10});
            }
            /*
            else if(buf.camId == 2){
                mean_bg_frame = whiteOutSameTone(cropped_background, mean_rgb, 80.0, {15,15,15});
            }
            * */
    
    bool detection;
    
    
        cv::Point2d difference;
        cv::Mat start_frame = mean_bg_frame.clone();
        cv::Mat tmp;
        
        
        
        cap.read(tmp);
        
        cv::Mat previous_frame = cropBetweenXs(tmp, leftX, rightX);
        
        cv::Point2d previous_difference = diffCentroidTol(start_frame, previous_frame, 15);
        
        
        
        
    // — 2) Sürekli okuma, kırpma ve paylaşılan arabellek —
    while (run) {
        cv::Mat frame;
        if (!cap.read(frame) || frame.empty()) break;
        Test = detectFirstVerticalLinesFromCenter(background); // çizgileri görmek için kullanıyoruz
        cv::Mat cropped = cropBetweenXs(frame, leftX, rightX);


            if(buf.camId == 0){
                mean_frame = whiteOutSameTone(cropped, mean_rgb, 80.0, {10,10,10});
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            else if(buf.camId == 1){
                mean_frame = whiteOutSameTone(cropped, mean_rgb, 80.0, {10,10,10});
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            /*
            else if(buf.camId == 2){
                mean_frame = whiteOutSameTone(cropped, mean_rgb, 80.0, {15,15,15});
                // detection = framesDifferAboveTol(mean_frame, mean_bg_frame, 15);
            }
            * */
            
            
            difference =  diffCentroidTol(mean_frame, previous_frame, 15);
            
            if (isCenterBetweenPoints(difference, previous_difference,rightX -leftX)){
                std::cout << "nesne ortada algılandı " << buf.camId << std::endl;
                buf.object_detection = true;
            }
            else{
                buf.object_detection = false;
            }
            
            previous_difference = difference;
            
            previous_frame = mean_frame.clone();
            
            
        

    
        // detection = framesDifferAbove(mean_frame, mean_bg_frame, 30);
       /* 
        if(detection){
        
            std::cout << "object is detected = Camera ID = " << buf.camId << std::endl;
        }
        else
        {
            std::cout << "image process ekibi" << buf.camId << std::endl;
        }
        */
        
        // detect_center_object(camId, buf, mean_bg_frame, mean_frame);

        
        // detect_moving_object_and_trigger(camId, buf, cropped);
        
        {
            std::lock_guard<std::mutex> lk(buf.m);
            buf.frame = std::move(mean_frame);
        }
        
    }

    if (--active_cameras == 0)
        all_cameras_done = true;
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
    // buffer2.camId = 2;
    
    std::thread t0(grabLoop, 0, std::ref(buffer0), std::ref(running),target_width, target_height);
    std::thread t1(grabLoop, 2, std::ref(buffer1), std::ref(running),target_width, target_height);
    // std::thread t2(grabLoop, 4, std::ref(buffer2), std::ref(running),target_width, target_height);
    
    cv::namedWindow("Cam0", cv::WINDOW_NORMAL);
    cv::namedWindow("Cam1", cv::WINDOW_NORMAL);
    //cv::namedWindow("Cam2", cv::WINDOW_NORMAL);
    cv::moveWindow("Cam0", 50, 50);
    cv::moveWindow("Cam1", 420, 50);
    //cv::moveWindow("Cam2", 640, 50);
    
    
    
    while (running) {
        
        char c = static_cast<char>(cv::waitKey(1));
        {
            std::lock_guard<std::mutex> lk(buffer0.m);
            if (!buffer0.frame.empty())
                cv::imshow("Cam0", buffer0.frame);
            
            if (buffer0.object_detection == true) {
            auto preprocessed_frame_item = create_preprocessed_frame_item(buffer0.frame, target_width, target_height);
            preprocessed_queue->push(preprocessed_frame_item);
            std::cout << "Frame alındı ve queue'ya eklendi." << std::endl;
            buffer0.object_detection = false;
            } 
            
        }
        {
            std::lock_guard<std::mutex> lk(buffer1.m);
            if (!buffer1.frame.empty())
                cv::imshow("Cam1", buffer1.frame);
            
            if (buffer1.object_detection == true){
            auto preprocessed_frame_item = create_preprocessed_frame_item(buffer1.frame, target_width, target_height);
            preprocessed_queue->push(preprocessed_frame_item);
            std::cout << "Frame alındı ve queue'ya eklendi." << std::endl;
            buffer1.object_detection = false;
            } 
            
        }
        /*
        {
            std::lock_guard<std::mutex> lk(buffer2.m);
            if (!buffer2.frame.empty())
                cv::imshow("Cam2", buffer2.frame);
            
            if (buffer2.object_detection == true){
            auto preprocessed_frame_item = create_preprocessed_frame_item(buffer2.frame, target_width, target_height);
            preprocessed_queue->push(preprocessed_frame_item);
            std::cout << "Frame alındı ve queue'ya eklendi." << std::endl;
            buffer2.object_detection = false;
            } 
            
        }
        */

        if (c == 27){
            running = false;
        }
    }

    t0.join();
    t1.join();
    // t2.join();
    
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

int main(int argc, char** argv)
{
    size_t class_count = 4; // 80 classes in COCO dataset
    double fps = 30;

    std::chrono::duration<double> inference_time;
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::high_resolution_clock::now();
    double org_height, org_width;
    cv::VideoCapture capture;
    size_t frame_count;
    InputType input_type;

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
                                class_count,
                                fps);

    hailo_status status = wait_and_check_threads(
        preprocess_thread,    "Preprocess",
        inference_thread,     "Inference",
        output_parser_thread, "Postprocess "
    );
    if (HAILO_SUCCESS != status) {
        return status;
    }

    if(!input_type.is_camera) {
        std::chrono::time_point<std::chrono::system_clock> t_end = std::chrono::high_resolution_clock::now();
        print_inference_statistics(inference_time, args.detection_hef, frame_count, t_end - t_start);
    }

    return HAILO_SUCCESS;
}




/*
void camera_continous_frames(uint32_t width, uint32_t height) {
    cv::VideoCapture cap(0); 
    if (!cap.isOpened()) {
        std::cerr << "Kamera açılamadı!" << std::endl;
        return;
    }
    
    control = 1;

    std::cout << "Kamera başlatıldı. Frame almak için 'q', çıkmak için 'ESC' tuşuna basın." << std::endl;

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Boş frame alındı!" << std::endl;
            continue;
        }

        cv::imshow("Kamera", frame);
        char c = (char)cv::waitKey(1);

        if (c == 'q' || c == 'Q') {
            auto preprocessed_frame_item = create_preprocessed_frame_item(frame, width, height);
            preprocessed_queue->push(preprocessed_frame_item);
            std::cout << "Frame alındı ve queue'ya eklendi." << std::endl;
        } 
        else if (c == 27) { // ESC key
            std::cout << "Kullanıcı çıkış yaptı.\n";
            active_cameras --;
            break;
        }
    }
    
    if(active_cameras == 0) {
        all_cameras_done = true;
    }

    cap.release();
    cv::destroyAllWindows();
    preprocessed_queue->stop(); // queue'yu durdur
    results_queue->stop(); // sonuç kuyruğunu da durdur
    control = 0;
}
*/
