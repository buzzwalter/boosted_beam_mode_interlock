#include "fourier_spectrum.hpp"
#include <fftw3.h>
#include <iostream>
#include <omp.h>
#include <cmath>

void FourierSpectrum::dftShift(cv::Mat& magnitude) {
    int cx = magnitude.cols / 2;
    int cy = magnitude.rows / 2;
    
    cv::Mat q0(magnitude, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(magnitude, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(magnitude, cv::Rect(0, cy, cx, cy)); 
    cv::Mat q3(magnitude, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);
}

cv::Mat FourierSpectrum::computeSpectrum(const cv::Mat& img) {
    // Convert to grayscale if not already
    cv::Mat gray;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img.clone();
    }

    // Convert to double precision
    gray.convertTo(gray, CV_64F);

    int rows = gray.rows;
    int cols = gray.cols;

    // Allocate FFTW arrays
    fftw_complex* fft_input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);
    fftw_complex* fft_output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);

    // Copy data to FFTW input
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fft_input[i * cols + j][0] = gray.at<double>(i,j);
            fft_input[i * cols + j][1] = 0;
        }
    }

    // Create and execute FFTW plan
    fftw_plan plan = fftw_plan_dft_2d(rows, cols, fft_input, fft_output, 
                                     FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    // Compute magnitude
    cv::Mat magnitude(rows, cols, CV_64F);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double real = fft_output[i * cols + j][0];
            double imag = fft_output[i * cols + j][1];
            magnitude.at<double>(i,j) = std::sqrt(real*real + imag*imag);
        }
    }

    // Apply power-law scaling
    cv::pow(magnitude, 0.2, magnitude);

    // Shift zero frequency to center
    dftShift(magnitude);

    // Log transform and normalize
    // magnitude += 1e-62;
    // cv::log(magnitude, magnitude);
    cv::normalize(magnitude, magnitude, 0, 255, cv::NORM_MINMAX);
    magnitude.convertTo(magnitude, CV_8U);

    // Cleanup
    fftw_destroy_plan(plan);
    fftw_free(fft_input);
    fftw_free(fft_output);
    
    return magnitude;
}

// double FourierSpectrum::computeBroadness(const cv::Mat& spectrum) {
//   return 0.0
// }

void FourierSpectrum::processImages(const std::vector<std::string>& image_paths, 
                                  const std::string& output_dir) {
    std::vector<cv::Mat> results(image_paths.size());

    #pragma omp parallel for
    for (size_t i = 0; i < image_paths.size(); ++i) {
        cv::Mat img = cv::imread(image_paths[i], cv::IMREAD_UNCHANGED);
        if (img.empty()) {
            std::cerr << "Failed to load image: " << image_paths[i] << std::endl;
            continue;
        }

        results[i] = computeSpectrum(img);
        std::cout << "Processed image " << i + 1 << "/" << image_paths.size() << std::endl;
    }

    // Save results
    for (size_t i = 0; i < results.size(); ++i) {
        std::string output_name = output_dir + "/spectrum_" + std::to_string(i) + ".png";
        cv::imwrite(output_name, results[i]);
        std::cout << "Saved spectrum: " << output_name << std::endl;
    }
}

