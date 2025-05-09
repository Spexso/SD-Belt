#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class CameraManager {
public:
    CameraManager(int camera_id, const std::string& window_name) 
        : camera_id(camera_id), window_name(window_name), running(true) {}
    
    void start() {
        // Thread'i başlat
        camera_thread = std::thread(&CameraManager::run, this);
    }
    
    void stop() {
        running = false;
        if (camera_thread.joinable()) {
            camera_thread.join();
        }
    }
    
    void run() {
        // Kamerayı aç
        cv::VideoCapture cap(camera_id);
        
        if (!cap.isOpened()) {
            std::cerr << "Kamera açılamadı: " << camera_id << std::endl;
            return;
        }
        
        std::cout << "Kamera " << camera_id << " başlatıldı" << std::endl;
        
        // Son frame alma zamanı
        auto last_capture_time = std::chrono::steady_clock::now();
        
        while (running) {
            cv::Mat frame;
            if (!cap.read(frame)) {
                std::cerr << "Kamera " << camera_id << " frame alamadı" << std::endl;
                break;
            }
            
            // Frame'i göster
            cv::imshow(window_name, frame);
            
            // Güncel zaman
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - last_capture_time).count();
            
            // Her 2 saniyede bir işlem yap
            if (elapsed >= 2) {
                std::cout << "Kamera " << camera_id << " - Frame alındı" << std::endl;
                last_capture_time = current_time;
                
                // Burada frame ile istediğiniz işlemleri yapabilirsiniz
            }
            
            // q tuşuna basılırsa çık
            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') {
                std::cout << "Kamera " << camera_id << " kapatılıyor..." << std::endl;
                break;
            }
        }
        
        // Kaynakları serbest bırak
        cap.release();
        cv::destroyWindow(window_name);
        std::cout << "Kamera " << camera_id << " kapatıldı" << std::endl;
    }
    
private:
    int camera_id;
    std::string window_name;
    std::thread camera_thread;
    std::atomic<bool> running;
};

int main() {
    // Kameraları tanımla
    CameraManager camera0(0, "Kamera 0");
    CameraManager camera1(2, "Kamera 2"); // İkinci kamera 2 numaralı olarak varsayılmıştır
    
    // Kameraları başlat
    camera0.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Threadler arasında kısa bir bekleme
    camera1.start();
    
    std::cout << "Tüm kameralar başlatıldı. Çıkmak için herhangi bir kamera penceresinde 'q' tuşuna basın." << std::endl;
    
    // Ana program kameraların kapatılmasını beklesin
    std::cout << "Çıkmak için herhangi bir tuşa basın..." << std::endl;
    std::cin.get();
    
    // Kameraları kapat
    camera0.stop();
    camera1.stop();
    
    std::cout << "Program sonlandı" << std::endl;
    return 0;
}
