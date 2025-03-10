// basler_camera.hpp
#ifndef BASLER_CAMERA_HPP
#define BASLER_CAMERA_HPP

// Standard library includes
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <vector>
#include <iostream>

// OpenCV includes
#include <opencv2/opencv.hpp>

// Pylon includes
#include <pylon/PylonIncludes.h>

class BaslerCamera {
public:
    BaslerCamera(size_t bufferSize = 10);
    ~BaslerCamera();

    // Camera control
    bool connect();
    void disconnect();
    bool startCapture();
    void stopCapture();
    bool configureCamera(double frameRate = 30.0);
    
    // Image retrieval
    bool getNextImage(cv::Mat& image);
    size_t getBufferSize() const;
    bool isConnected() const;
    bool isCapturing() const;

    // Camera settings
    void setExposureTime(double microseconds);
    void setGain(double gain);
    double getExposureTime() const;
    double getGain() const;

private:
    // Pylon camera instance
    Pylon::CInstantCamera camera;
    bool connected;
    bool capturing;
    
    // Image buffer
    size_t maxBufferSize;
    std::queue<cv::Mat> imageBuffer;
    mutable std::mutex bufferMutex;  // Mark as mutable so it can be locked in const methods
    std::condition_variable bufferCondition;
    
    // Capture thread
    std::thread captureThread;
    bool stopThread;
    
    // Private methods
    void captureLoop();
    void clearBuffer();
};

#endif // BASLER_CAMERA_HPP
