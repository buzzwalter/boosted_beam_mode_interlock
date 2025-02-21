#ifndef FOURIER_SPECTRUM_HPP
#define FOURIER_SPECTRUM_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class FourierSpectrum {
public:
    // Main processing functions
    static cv::Mat computeSpectrum(const cv::Mat& img);
    static void processImages(const std::vector<std::string>& image_paths, const std::string& output_dir = "./data");    
    static double computeBroadness(const cv::Mat& spectrum);

private:
    static void dftShift(cv::Mat& magnitude);
    static cv::Mat createDistanceMatrix(int rows, int cols);
    static vector<double> computerRadialProfile(const cv::Mat& magnitude, const cv::Mat& distances, int num_bins);
};

#endif // FOURIER_SPECTRUM_HPP
