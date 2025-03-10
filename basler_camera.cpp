// basler_camera.cpp
#include "basler_camera.hpp"

using namespace Pylon;
// Include the necessary parameter headers
#include <pylon/ParameterIncludes.h>

BaslerCamera::BaslerCamera(size_t bufferSize)
    : maxBufferSize(bufferSize)
    , connected(false)
    , capturing(false)
    , stopThread(false) {
    // Initialize Pylon runtime
    PylonInitialize();
}

BaslerCamera::~BaslerCamera() {
    disconnect();
    // Terminate Pylon runtime
    PylonTerminate();
}

bool BaslerCamera::connect() {
    try {
        // Get the transport layer factory
        CTlFactory& tlFactory = CTlFactory::GetInstance();

        // Get all attached devices and select first device
        DeviceInfoList_t devices;
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
    catch (const GenericException& e) {
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

bool BaslerCamera::configureCamera(double frameRate) {
    if (!connected) return false;
    
    try {
        // Get the camera's node map
        INodeMap& nodemap = camera.GetNodeMap();
        
        // Configure frame rate
        GenApi::CBooleanPtr frameRateEnable = nodemap.GetNode("AcquisitionFrameRateEnable");
        if (frameRateEnable && GenApi::IsWritable(frameRateEnable)) {
            frameRateEnable->SetValue(true);
        }
        
        GenApi::CFloatPtr frameRateNode = nodemap.GetNode("AcquisitionFrameRate");
        if (frameRateNode && GenApi::IsWritable(frameRateNode)) {
            frameRateNode->SetValue(frameRate);
        }
        
        // Turn off auto-exposure for consistent images
        GenApi::CEnumerationPtr exposureAuto = nodemap.GetNode("ExposureAuto");
        if (exposureAuto && GenApi::IsWritable(exposureAuto)) {
            exposureAuto->FromString("Off");
        }
        
        // Set pixel format to Mono8 for 8-bit grayscale
        GenApi::CEnumerationPtr pixelFormat = nodemap.GetNode("PixelFormat");
        if (pixelFormat && GenApi::IsWritable(pixelFormat)) {
            pixelFormat->FromString("Mono8");
        }
        
        return true;
    }
    catch (const GenericException& e) {
        std::cerr << "Failed to configure camera: " << e.GetDescription() << std::endl;
        return false;
    }
}

bool BaslerCamera::startCapture() {
    if (!connected || capturing) {
        return false;
    }

    try {
        // Start grabbing
        camera.StartGrabbing(GrabStrategy_LatestImageOnly);
        
        // Start capture thread
        stopThread = false;
        captureThread = std::thread(&BaslerCamera::captureLoop, this);
        capturing = true;
        return true;
    }
    catch (const GenericException& e) {
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
    CGrabResultPtr grabResult;
    
    while (!stopThread && camera.IsGrabbing()) {
        try {
            // Wait for an image and grab it
            camera.RetrieveResult(5000, grabResult, TimeoutHandling_ThrowException);
            
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
        catch (const TimeoutException&) {
            // Timeout - continue
            continue;
        }
        catch (const GenericException& e) {
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
            // Get the camera's node map
            INodeMap& nodemap = camera.GetNodeMap();
            
            // Set exposure time using GenAPI directly
            GenApi::CFloatPtr exposureTime = nodemap.GetNode("ExposureTimeAbs");
            if (exposureTime && GenApi::IsWritable(exposureTime)) {
                exposureTime->SetValue(microseconds);
            }
        }
        catch (const GenericException& e) {
            std::cerr << "Failed to set exposure time: " << e.GetDescription() << std::endl;
        }
    }
}

void BaslerCamera::setGain(double gain) {
    if (connected) {
        try {
            // Get the camera's node map
            INodeMap& nodemap = camera.GetNodeMap();
            
            // Set gain using GenAPI directly
            GenApi::CIntegerPtr gainNode = nodemap.GetNode("GainRaw");
            if (gainNode && GenApi::IsWritable(gainNode)) {
                gainNode->SetValue(static_cast<int>(gain));
            }
        }
        catch (const GenericException& e) {
            std::cerr << "Failed to set gain: " << e.GetDescription() << std::endl;
        }
    }
}

double BaslerCamera::getExposureTime() const {
    if (connected) {
        try {
            // Using const_cast because GetNodeMap() is not const in the API
            // This is a common issue with many C++ APIs
            INodeMap& nodemap = const_cast<CInstantCamera&>(camera).GetNodeMap();
            
            // Get exposure time using GenAPI directly
            GenApi::CFloatPtr exposureTime = nodemap.GetNode("ExposureTimeAbs");
            if (exposureTime && GenApi::IsReadable(exposureTime)) {
                return exposureTime->GetValue();
            }
        }
        catch (const GenericException& e) {
            std::cerr << "Failed to get exposure time: " << e.GetDescription() << std::endl;
        }
    }
    return 0.0;
}

double BaslerCamera::getGain() const {
    if (connected) {
        try {
            // Using const_cast because GetNodeMap() is not const in the API
            INodeMap& nodemap = const_cast<CInstantCamera&>(camera).GetNodeMap();
            
            // Get gain using GenAPI directly
            GenApi::CIntegerPtr gainNode = nodemap.GetNode("GainRaw");
            if (gainNode && GenApi::IsReadable(gainNode)) {
                return static_cast<double>(gainNode->GetValue());
            }
        }
        catch (const GenericException& e) {
            std::cerr << "Failed to get gain: " << e.GetDescription() << std::endl;
        }
    }
    return 0.0;
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