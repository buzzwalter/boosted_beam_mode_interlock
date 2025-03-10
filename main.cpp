#include "fourier_spectrum.hpp"
#include "basler_camera.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <csignal>
#include <fstream>

namespace fs = std::filesystem;

std::vector<double> broadnessValues;
std::vector<cv::Mat> images;

void saveData(const std::vector<double>& broadnessValues, const std::vector<cv::Mat>& images) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream folderName;
    folderName << "./data/" << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");

    fs::create_directories(folderName.str());

    std::ofstream broadnessFile(folderName.str() + "/broadness_values.txt");
    for (double value : broadnessValues) {
        broadnessFile << value << "\n";
    }

    for (size_t i = 0; i < images.size(); ++i) {
        std::ostringstream imageName;
        imageName << folderName.str() << "/image_" << i << ".png";
        cv::imwrite(imageName.str(), images[i]);
    }
}

void signalHandler(int signal) {
    std::cout << "\nInterrupt received, saving data..." << std::endl;
    saveData(broadnessValues, images);
    std::exit(0);
}

int main() {
    std::signal(SIGINT, signalHandler);

    BaslerCamera camera(10);
    if (!camera.connect()) {
        std::cerr << "Failed to connect to camera" << std::endl;
        return 1;
    }

    camera.setExposureTime(10000);  // 10ms exposure
    camera.startCapture();

    cv::Mat image;
    
    int i = 0;
    while (true) {
        if (camera.getNextImage(image)) {
            std::cout << "Processing image " << i << std::endl;
            cv::Mat spectrum = FourierSpectrum::computeSpectrum(image);
            double broadness = FourierSpectrum::computeBroadness(spectrum, 371);
            std::cout << "Image broadness: " << broadness << std::endl;

            std::pair<std::vector<double>,int> smoothedDataAndIndicator;
            if (i > 100){
              smoothedDataAndIndicator = FourierSpectrum::calculateSmoothedDataAndIndicator(broadnessValues, 30, 2.0);
              if(smoothedDataAndIndicator.second) {
                std::cout << "Damage detected!" << std::endl;
                saveData(broadnessValues, images);
                std::exit(0);
              }
            }
            if (broadnessValues.size() >= 100) {
                broadnessValues.erase(broadnessValues.begin());
                images.erase(images.begin());
            }
            broadnessValues.push_back(broadness);
            images.push_back(image.clone());
            ++i;
        }
    }

    camera.stopCapture();
    camera.disconnect();
    return 0;
}
