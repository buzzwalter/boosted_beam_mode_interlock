#include "fourier_spectrum.hpp"
#include "basler_camera.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

void saveData(const std::vector<double>& broadnessValues, const std::vector<cv::Mat>& images) {
    // Create a timestamped folder
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream folderName;
    folderName << "./data/" << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");

    fs::create_directories(folderName.str());

    // Save broadness values
    std::ofstream broadnessFile(folderName.str() + "/broadness_values.txt");
    for (double value : broadnessValues) {
        broadnessFile << value << "\n";
    }

    // Save images
    for (size_t i = 0; i < images.size(); ++i) {
        std::ostringstream imageName;
        imageName << folderName.str() << "/image_" << i << ".png";
        cv::imwrite(imageName.str(), images[i]);
    }
}

int main() {
    BaslerCamera camera(10);  // Buffer 10 images

    if (!camera.connect()) {
        std::cerr << "Failed to connect to camera" << std::endl;
        return 1;
    }

    camera.setExposureTime(10000);  // 10ms exposure
    camera.startCapture();

    std::vector<double> broadnessValues;
    std::vector<cv::Mat> images;

    cv::Mat image;
    int i = 0;
    while (true) {
        if (camera.getNextImage(image)) {
            std::cout << "Processing image " << i << std::endl;
            cv::Mat spectrum = FourierSpectrum::computeSpectrum(image);
            double broadness = FourierSpectrum::computeBroadness(spectrum, 371);
            std::cout << "Image broadness: " << broadness << std::endl;

            // Add data to the queues
            if (broadnessValues.size() >= 100) {
                broadnessValues.erase(broadnessValues.begin());
                images.erase(images.begin());
            }
            broadnessValues.push_back(broadness);
            images.push_back(image.clone());

            ++i;
        }

        // Exit condition
        if (cv::waitKey(10) == 27) {  // Press 'Esc' key to exit
            saveData(broadnessValues, images);
            break;
        }
    }

    camera.stopCapture();
    camera.disconnect();
    return 0;
}
