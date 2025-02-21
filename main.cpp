#include "fourier_spectrum.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <thread>


int main() {
  // std::vector<std::string> image_paths = {
  //   "./data/test_images/0009.tiff",
  //   "./data/test_images/0150.tiff",
  //   "./data/test_images/0260.tiff"   
  // };
  std::vector<std::string> image_paths;
  for (const auto& file : std::filesystem::directory_iterator("./data/test_images")){
    // std::cout << file.path().string() << std::endl;
    image_paths.emplace_back(file.path().string());
  }
  
  //  std::cout << "no seggy here" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Introduce a short delay


  FourierSpectrum::processImages(image_paths);
  return 0;
}
