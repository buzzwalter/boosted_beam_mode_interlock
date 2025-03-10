#include "fourier_spectrum.hpp"
#include "basler_camera.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
int main() {
  BaslerCamera camera(10);  // Buffer 10 images
  
  if (!camera.connect()) {
      std::cerr << "Failed to connect to camera" << std::endl;
      return 1;
  }
  
  camera.setExposureTime(10000);  // 10ms exposure
  camera.startCapture();
  
  // Process images in a loop
  cv::Mat image;
  int i = 0;
  while (true) {
      if (camera.getNextImage(image)) {
          std::cout << "Processing image " << i << std::endl;
          cv::Mat spectrum = FourierSpectrum::computeSpectrum(image);
          double broadness = FourierSpectrum::computeBroadness(spectrum, 371);
          std::cout << "Image broadness: " << broadness << std::endl;

          // Do something with the results
      }
  }
  
  camera.stopCapture();
  camera.disconnect();
  return 0;
}

// #include "fourier_spectrum.hpp"
// #include "basler_camera.hpp"
// #include <iostream>
// #include <vector>
// #include <string>

// int main() {
//     // Option 1: Use the BaslerCamera for live capture
//     BaslerCamera camera(10);  // Buffer 10 images
    
//     if (camera.connect()) {
//         std::cout << "Connected to camera" << std::endl;
        
//         // Configure camera with 30 fps frame rate
//         if (camera.configureCamera(30.0)) {
//             std::cout << "Camera configured successfully" << std::endl;
//         }
        
//         camera.setExposureTime(10000);  // 10ms exposure
        
//         if (camera.startCapture()) {
//             std::cout << "Started capture" << std::endl;
            
//             // Process a few frames
//             for (int i = 0; i < 10; i++) {
//                 cv::Mat image;
//                 if (camera.getNextImage(image)) {
//                     std::cout << "Processing image " << i << std::endl;
//                     cv::Mat spectrum = FourierSpectrum::computeSpectrum(image);
//                     double broadness = FourierSpectrum::computeBroadness(spectrum, 371);
//                     std::cout << "Image broadness: " << broadness << std::endl;
//                 }
//             }
            
//             camera.stopCapture();
//         }
//         camera.disconnect();
//     }
    
//     // Option 2: Process existing images from disk
//     // std::vector<std::string> image_paths = {
//     //     "./data/test_images/0009.tiff",
//     //     "./data/test_images/0150.tiff",
//     //     "./data/test_images/0260.tiff"
//     // };
    
//     // FourierSpectrum::processImages(image_paths);
    
//     return 0;
// }