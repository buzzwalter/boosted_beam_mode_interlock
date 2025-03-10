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
    static void binningAnalysis(const std::vector<std::string>& image_paths, int bin_minimum, int bin_maximum,  const std::string& output_dir = "./data");
    static double computeBroadness(const cv::Mat& spectrum, int num_bins);
    static std::pair<std::vector<double>,int> calculateSmoothedDataAndIndicator(const std::vector<double>& data, int smoothingWindow, double thresholdSigma);

private:
    static void dftShift(cv::Mat& magnitude);
    static cv::Mat createDistanceMatrix(int rows, int cols);
    static std::vector<double> computeRadialProfile(const cv::Mat& magnitude, const cv::Mat& distances, int num_bins);
    static std::vector<double> smoothData(const std::vector<double>& data, int windowSize);
    static std::vector<double> calculateDerivative(const std::vector<double>& data);
  };

#endif // FOURIER_SPECTRUM_HPP
