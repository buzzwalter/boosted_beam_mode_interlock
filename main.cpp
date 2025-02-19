#include "fourier_spectrum.hpp"
#include <vector>
#include <string>

int main() {
  std::vector<std::string> image_paths = {
    "./data/test_images/0009.tiff",
    "./data/test_images/0150.tiff",
    "./data/test_images/0260.tiff"   
  };
  
  FourierSpectrum::processImages(image_paths);
  return 0;
}
