// basler_camera.cpp
#include "basler_camera.hpp"
#include <iostream>

BaslerCamera::BaslerCamera(size_t bufferSize)
    : maxBufferSize(bufferSize)
    , connected(false)
    , capturing(false)
    , stopThread(false) {
    // Initialize Pylon runtime
    Pylon::PylonInitialize();
}

BaslerCamera::~BaslerCamera() {
    disconnect();
    // Terminate Pylon runtime
    Pylon::PylonTerminate();
}

bool BaslerCamera::connect() {
    try {
        // Get the transport layer factory
        Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

        // Get all attached devices and select first device
        Pylon::DeviceInfoList_t devices;
        if (tlFactory.EnumerateDevices(devices) == 0) {
            std::cerr << "No camera found." << std::endl;
            return false;
        }

        // Create camera object
        camera.Attach(tlFactory.CreateDevice(devices[0]));
        camera.Open();
        
        // Set default camera settings
        camera.MaxNumBuffer = maxBufferSize;
        
        connected = true;
        return true;
    }
    catch (const Pylon::GenericException& e) {
        std::cerr << "Camera connection failed: " << e.GetDescription() << std::endl;
        return false;
    }
}

void BaslerCamera::disconnect() {
    stopCapture();
    if (connected) {
        camera.Close();
        connected = false;
    }
}

bool BaslerCamera::startCapture() {
    if (!connected || capturing) {
        return false;
    }

    try {
        // Start grabbing
        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly);
        
        // Start capture thread
        stopThread = false;
        captureThread = std::thread(&BaslerCamera::captureLoop, this);
        capturing = true;
        return true;
    }
    catch (const Pylon::GenericException& e) {
        std::cerr << "Failed to start capture: " << e.GetDescription() << std::endl;
        return false;
    }
}

void BaslerCamera::stopCapture() {
    if (capturing) {
        stopThread = true;
        bufferCondition.notify_all();
        if (captureThread.joinable()) {
            captureThread.join();
        }
        camera.StopGrabbing();
        capturing = false;
        clearBuffer();
    }
}

void BaslerCamera::captureLoop() {
    Pylon::CGrabResultPtr grabResult;
    
    while (!stopThread && camera.IsGrabbing()) {
        try {
            // Wait for an image and grab it
            camera.RetrieveResult(5000, grabResult, Pylon::TimeoutHandling_ThrowException);
            
            if (grabResult->GrabSucceeded()) {
                // Convert to OpenCV Mat
                cv::Mat image(grabResult->GetHeight(), grabResult->GetWidth(), CV_8UC1,
                            (uint8_t*)grabResult->GetBuffer());
                
                // Add to buffer
                std::unique_lock<std::mutex> lock(bufferMutex);
                if (imageBuffer.size() >= maxBufferSize) {
                    imageBuffer.pop(); // Remove oldest image if buffer is full
                }
                imageBuffer.push(image.clone());
                lock.unlock();
                bufferCondition.notify_one();
            }
        }
        catch (const Pylon::TimeoutException&) {
            // Timeout - continue
            continue;
        }
        catch (const Pylon::GenericException& e) {
            std::cerr << "Capture error: " << e.GetDescription() << std::endl;
            break;
        }
    }
}

bool BaslerCamera::getNextImage(cv::Mat& image) {
    std::unique_lock<std::mutex> lock(bufferMutex);
    
    // Wait for image if buffer is empty
    if (imageBuffer.empty()) {
        bufferCondition.wait_for(lock, std::chrono::seconds(5));
        if (imageBuffer.empty()) {
            return false;
        }
    }
    
    image = imageBuffer.front();
    imageBuffer.pop();
    return true;
}

void BaslerCamera::clearBuffer() {
    std::unique_lock<std::mutex> lock(bufferMutex);
    std::queue<cv::Mat> empty;
    std::swap(imageBuffer, empty);
}

void BaslerCamera::setExposureTime(double microseconds) {
    if (connected) {
        try {
            camera.ExposureTime.SetValue(microseconds);
        }
        catch (const Pylon::GenericException& e) {
            std::cerr << "Failed to set exposure time: " << e.GetDescription() << std::endl;
        }
    }
}

void BaslerCamera::setGain(double gain) {
    if (connected) {
        try {
            camera.Gain.SetValue(gain);
        }
        catch (const Pylon::GenericException& e) {
            std::cerr << "Failed to set gain: " << e.GetDescription() << std::endl;
        }
    }
}

double BaslerCamera::getExposureTime() const {
    return connected ? camera.ExposureTime.GetValue() : 0.0;
}

double BaslerCamera::getGain() const {
    return connected ? camera.Gain.GetValue() : 0.0;
}

size_t BaslerCamera::getBufferSize() const {
    std::unique_lock<std::mutex> lock(bufferMutex);
    return imageBuffer.size();
}

bool BaslerCamera::isConnected() const {
    return connected;
}

bool BaslerCamera::isCapturing() const {
    return capturing;
}