#include "fourier_spectrum.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>

int main() {
  // std::vector<std::string> image_paths = {
  //   "./data/test_images/0009.tiff",
  //   "./data/test_images/0150.tiff",
  //   "./data/test_images/0260.tiff"   
  // };

  std::vector<std::string> image_paths;

  // Loop through test_images
  for (int i = 50; i < 80; i++){
    // std::cout << file.path().string() << std::endl;
    std::ostringstream oss;
    oss << "./data/test_images" << "/" << std::setw(4) << std::setfill('0') << i << ".tiff";
    image_paths.push_back(oss.str());
    // image_paths.emplace_back(std::format("{}{:04}{}","./data/test_images/",i,".tiff"));
  }
  
  //  std::cout << "no seggy here" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(30));  // Introduce a short delay


  // FourierSpectrum::processImages(image_paths);
  FourierSpectrum::binningAnalysis(image_paths, 30, 40);
  return 0;
}
