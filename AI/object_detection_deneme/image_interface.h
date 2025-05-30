#ifndef IMAGE_INTERFACE
#define IMAGE_INTERFACE

#include <opencv2/core.hpp>
#include <cstddef>
#include <string>

/**
 * Pure-configuration interface that holds project-wide constants.
 * Edit the literal values below to suit your application.
 */
class ImageInterface
{
public:
    /* --- numeric parameters ------------------------------------------------ */
    
    
    inline static const std::string DESKTOP_IP_UDP   {"192.168.97.229"};
    inline static const std::string SERVER_IP   {"192.168.97.229"};
    inline static const std::string ARDUINO_PORT {"/dev/ttyUSB0"};
    inline static const std::string InoFilePath = "../../Hardware/SerialPort_communication/SerialPort_communication.ino";
    inline static const std::string BACKEND_SCANS_POINT {"/api/v1/scans"};
	inline static const std::string BACKEND_SYSTEMINFO_POINT = "/api/v1/system/info";
	inline static const std::string BACKEND_SYSTEMMESSAGE_POINT = "/api/v1/system/logs";
    inline static constexpr int  BACKEND_PORT = 6060;
    inline static constexpr int UDP_COMMS_PORT = 5000;
    
    inline static constexpr int        SAVE_NUMBER          = 0;
    inline static constexpr int        CAMERA_NUMBER        = 3;     // active camera index
    inline static constexpr int        COOLDOWN_SECONDS     = 5;     // cooldown between captures
    inline static constexpr std::size_t QUEUE_SIZE          = 60;    // ring-buffer length
    inline static constexpr double     LUMIN_TOL_PERCENT    = 60;  
    inline static constexpr double     DIFFER_LUMIN_TOL_REFERENCE = 60;
	inline static constexpr double 	   THRESHOLD_DIFFERENCE = 10;
	inline static constexpr double 	   THRESHOLD_DIFFERENCE_0 = 10;
	inline static constexpr double 	   THRESHOLD_DIFFERENCE_1 = 10;
	inline static constexpr double 	   THRESHOLD_DIFFERENCE_2 = 5;

	inline static constexpr double     CENTER_POINT_RATIO	= 0.30;
 	inline static constexpr int STARTUP_DELAY_SECONDS = 3;   // Seko başlangıç delay

    /* --- colour-tolerance parameters --------------------------------------- */
    // Per-channel (R,G,B) tolerance in percent, expressed as a Vec3d
    inline static const cv::Vec3d COLOR_TOL_PERCENT_RGB           {7,7,7};
    inline static const cv::Vec3d DIFFER_COLOR_TOL_PERCENT_RGB    {7,7,7};
    

protected:
    // No instances – this class is just a namespace-like container.
    ImageInterface()  = default;
    ~ImageInterface() = default;
};

#endif // IMAGE_INTERFACE
