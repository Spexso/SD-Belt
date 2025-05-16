#ifndef IMAGE_INTERFACE
#define IMAGE_INTERFACE

#include <opencv2/core.hpp>
#include <cstddef>

/**
 * Pure-configuration interface that holds project-wide constants.
 * Edit the literal values below to suit your application.
 */
class ImageInterface
{
public:
    /* --- numeric parameters ------------------------------------------------ */
    inline static constexpr int        CAMERA_NUMBER        = 2;     // active camera index
    inline static constexpr int        COOLDOWN_SECONDS     = 3;     // cooldown between captures
    inline static constexpr std::size_t QUEUE_SIZE          = 60;    // ring-buffer length
    inline static constexpr double     LUMIN_TOL_PERCENT    = 50;  
    inline static constexpr double     DIFFER_LUMIN_TOL_REFERENCE = 60;
	inline static constexpr double 	   THRESHOLD_DIFFERENCE = 15;
    /* --- colour-tolerance parameters --------------------------------------- */
    // Per-channel (R,G,B) tolerance in percent, expressed as a Vec3d
    inline static const cv::Vec3d COLOR_TOL_PERCENT_RGB           {5,5,5};
    inline static const cv::Vec3d DIFFER_COLOR_TOL_PERCENT_RGB    {5,5,5};
    

protected:
    // No instances – this class is just a namespace-like container.
    ImageInterface()  = default;
    ~ImageInterface() = default;
};

#endif // IMAGE_INTERFACE
