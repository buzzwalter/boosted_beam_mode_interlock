#include <iostream>
#include <ostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp> // For image processing
#include <fftw3.h>           // For Fourier Transform
#include <omp.h>             // For multi-threading

void dftShift(cv::Mat& magnitude){
    int cx = magnitude.cols / 2;
    int cy = magnitude.rows / 2;
    
    cv::Mat q0(magnitude, cv::Rect(0,0,cx,cy));
    cv::Mat q1(magnitude, cv::Rect(cx,0,cx,cy));
    cv::Mat q2(magnitude, cv::Rect(0,cy,cx,cy)); 
    cv::Mat q3(magnitude, cv::Rect(cx,cy,cx,cy));

    cv::Mat tmp;
    //    q3.copyTo(tmp); q0.copyTo(q3); tmp.copyTo(q0);
    //    q2.copyTo(tmp); q1.copyTo(q2); tmp.copyTo(q2);
    //    q3.copyTo(tmp); q0.copyTo(q3);
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    //    q2.copyTo(tmp); q1.copyTo(q2);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);
    
}

// Function to compute the Fourier spectrum of an image
cv::Mat compute_fourier_spectrum(const cv::Mat& img) {
    // Convert to grayscale if not already
    cv::Mat gray;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img.clone();
    }

    // Convert grayscale image to double precision
    gray.convertTo(gray, CV_64F);

    // Get the image dimensions
    int rows = gray.rows;
    int cols = gray.cols;

    // Allocate FFTW input and output arrays
    fftw_complex* fft_input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);
    fftw_complex* fft_output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * rows * cols);

    // std::memcpy(fft_input, gray.data ,sizeof(double) * rows * cols);
    for (int i=0; i < rows; i++){
      for (int j=0; j < cols; j++){
	fft_input[i * cols + j][0] = gray.at<double>(i,j);
	fft_input[i * cols + j][1] = 0;
      }
    }

    // Create FFTW plan with kinds and execute
    // fftwf_r2r_kind kinds[] = {FFTW_R2HC, FFTW_R2HC};
    fftw_plan plan = fftw_plan_dft_2d(rows, cols, fft_input, fft_output, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);


    // Compute the magnitude (absolute value) of the spectrum
    cv::Mat magnitude(rows, cols, CV_64F, fft_output);

    for (int i=0; i < rows; i++){
      for (int j=0; j < cols; j++){
	double real = fft_output[i * cols + j][0];
	double imag = fft_output[i * cols + j][1];
	magnitude.at<double>(i,j) = std::sqrt(real*real + imag*imag);
      }
    }

    //    std::cout << "fft_output at index 0,0: " << magnitude.at<double>(0,0) << std::endl;

    // Apply power-law scaling (0.2)
    cv::pow(magnitude, 0.2, magnitude);


    // Shift zero freq. to center
    dftShift(magnitude);

    double minVal, maxVal;
    cv::minMaxLoc(magnitude, &minVal, &maxVal);
    std::cout << "Before normalization: min=" << minVal << ", max=" << maxVal << std::endl;
    
    // Log transform to prevent clipping
    magnitude += 1e-62;
    cv::log(magnitude, magnitude);

    cv::minMaxLoc(magnitude, &minVal, &maxVal);
    std::cout << "Before normalization(after log): min=" << minVal << ", max=" << maxVal << std::endl;

    // Normalize the spectrum to [0, 1]
    cv::normalize(magnitude, magnitude, 0, 255, cv::NORM_MINMAX);
    
    magnitude.convertTo(magnitude, CV_8U);

    // Free FFTW resources
    fftw_destroy_plan(plan);
    fftw_free(fft_input);
    fftw_free(fft_output);
    
    return magnitude;
}

// Multi-threaded processing of multiple images
void process_images(const std::vector<std::string>& image_paths) {
    std::vector<cv::Mat> results(image_paths.size());

    // Parallelize the loop using OpenMP
    #pragma omp parallel for
    for (size_t i = 0; i < image_paths.size(); ++i) {
        // Load the image
        cv::Mat img = cv::imread(image_paths[i], cv::IMREAD_UNCHANGED);
        if (img.empty()) {
            std::cerr << "Failed to load image: " << image_paths[i] << std::endl;
	    continue;
        }

        // Compute the Fourier spectrum
        results[i] = compute_fourier_spectrum(img);
        std::cout << "Processed image " << i + 1 << "/" << image_paths.size() << std::endl;
    }

    // Save or display results (optional)
    for (size_t i = 0; i < results.size(); ++i) {
        //std::cout << "we get this far" << std::endl;
        std::string output_name = "./data/spectrum_" + std::to_string(i) + ".png";
        cv::imwrite(output_name, results[i]); // Save as 8-bit image
        std::cout << "Saved spectrum: " << output_name << std::endl;
    }
}

int main() {
    // List of image paths (replace with your own paths)
    std::vector<std::string> image_paths = {
        "./data/test_images/0009.tiff", "./data/test_images/0150.tiff", "./data/test_images/0260.tiff", // Add all 292 paths here
    };


    // Process images
     process_images(image_paths);

    return 0;
}
