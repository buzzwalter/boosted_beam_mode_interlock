#include "fourier_spectrum.hpp"
#include <fftw3.h>
#include <iostream>
#include <numeric>
#include <omp.h>
#include <cmath>
#include <fstream>

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
    //magnitude.convertTo(magnitude, CV_8U);

    // Cleanup
    fftw_destroy_plan(plan);
    fftw_free(fft_input);
    fftw_free(fft_output);
    
    return magnitude;
}

cv::Mat FourierSpectrum::createDistanceMatrix(int rows, int cols){
   cv::Mat distances(rows,cols,CV_64F);
   int center_y = rows / 2;
   int center_x = cols / 2; 
   for (int i = 0; i < rows; i++){
     for (int j = 0; j < cols; j++) {
       distances.at<double>(i,j) = std::sqrt((j-center_x) * (j-center_x) + (i-center_y) * (i-center_y));
     }
   }
   
   double max;
   double min;
   cv::minMaxLoc(distances, &min, &max);
   // Normalize distances
   distances = distances / max;

   return distances;
}

std::vector<double> FourierSpectrum::computeRadialProfile(const cv::Mat& magnitude, const cv::Mat& distances, int num_bins){

  std::vector<double> radial_profile(num_bins, 0.0);
  std::vector<int> bin_counts(num_bins, 0);

  // Set bin edge distances (remember that they're normalized)
  std::vector<double> bin_edges(num_bins + 1);
  for (int i = 0; i <= num_bins; i++){
    bin_edges[i] = static_cast<double>(i) / num_bins;
  }

  // Populate buckets
  for (int i = 0; i < distances.rows; i++){
    for (int j = 0; j < distances.cols; j++){
      double dist = distances.at<double>(i,j);
      double mag = magnitude.at<double>(i,j);
      
      // Find the bin and break loop to save unnecessary jumps
      // for (int b = 0; b < num_bins; b++){
      // 	if (dist >= bin_edges[b] && dist <= bin_edges[b+1]){
      // 	  radial_profile[b] += mag;
      // 	  bin_counts[b]++;
      // 	  break;
      // 	}
      // }
      int bin_index = std::lower_bound(bin_edges.begin(), bin_edges.end(), dist) - bin_edges.begin() - 1;

      // Ensure bin index is valid
      if (bin_index >= 0 && bin_index < num_bins) {
	radial_profile[bin_index] += mag;
	bin_counts[bin_index]++;
      }

    }
  }
  
  // Build dsn. by calculating frequency
  for (int b = 0; b < num_bins; b++){
    if(bin_counts[b] > 0){
      //      std::cout << radial_profile[b] << " " << bin_counts[b] << std::endl;
      radial_profile[b] = radial_profile[b] / bin_counts[b];

    }
  }
  
  // Clean it up just in case there are NaN values -- might remove or comment out
  std::vector<double> cleaned_profile;
  for (int b = 0; b < num_bins; b++){
    if (bin_counts[b] >= 0){
      cleaned_profile.emplace_back(radial_profile[b]);
    }
  }
  
  return cleaned_profile;
}


double FourierSpectrum::computeBroadness(const cv::Mat& spectrum) {

  // Create distance matrix
  cv::Mat distances_normalized = createDistanceMatrix(spectrum.rows, spectrum.cols);
  
  // Normalize magnitude
  cv::Mat magnitude_normalized;
  double max;
  double min;
  cv::minMaxLoc(spectrum, &min, &max);
  magnitude_normalized =  (spectrum - min) / (max - min);
  
  //  std::cout << "min and max: " << min << " " << max << std::endl;


  // Compute the radial profile with bin number of 50 -- could try different bin numbers
  int num_bins = 100;
  std::vector<double> radial_profile = computeRadialProfile(magnitude_normalized, distances_normalized, num_bins);

  // First calculate mean for std
  double sum = std::accumulate(radial_profile.begin(),radial_profile.end(),0.0);
  double mean = sum / radial_profile.size();

  // Calculate std
  double sq_sum = std::accumulate(radial_profile.begin(), radial_profile.end(), 0.0, [mean](double acc, double val) {
    return acc + (val - mean) * (val - mean);
  });
  double radial_std = std::sqrt(sq_sum) / (radial_profile.size() - 1); // could replace with bin_size if certain no NaN values can appear
  
  return radial_std;
}

void FourierSpectrum::processImages(const std::vector<std::string>& image_paths, 
                                  const std::string& output_dir) {
    std::vector<cv::Mat> results(image_paths.size());
    std::vector<double> radial_stds(107, 0.0);
   
    double radial_std;
    // Uncomment for parralelism 
    // #pragma omp parallel for
    for (size_t i = 0; i < image_paths.size(); ++i) {
        cv::Mat img = cv::imread(image_paths[i], cv::IMREAD_UNCHANGED);
        if (img.empty()) {
            std::cerr << "Failed to load image: " << image_paths[i] << std::endl;
            continue;
        }

        results[i] = computeSpectrum(img);
        std::cout << "Processed image " << i << "/" << image_paths.size() << std::endl;
	radial_std = computeBroadness(results[i]);
	radial_stds[i] = radial_std;
	std::cout << radial_std << std::endl;
	
    }
    
    // Save results
    for (size_t i = 0; i < results.size(); ++i) {
        std::string output_name = output_dir + "/spectrum_" + std::to_string(i) + ".png";
	results[i].convertTo(results[i],CV_8U);
        cv::imwrite(output_name, results[i]);
        std::cout << "Saved spectrum: " << output_name << std::endl;
    }

    // Open the file in output mode (create or overwrite)
    std::ofstream out_file("./data/radial_stds.txt");

    // Check if the file was opened successfully
    if (!out_file) {
        std::cerr << "Error opening file for writing!" << std::endl;
    }

    // Write the vector to the file (one number per line, or space-separated)
    for (int i = 0; i < image_paths.size(); i++) {
        out_file << image_paths[i] << " " << radial_stds[i]  << "\n";  // Write each value on a new line
        // Alternatively, use: out_file << val << " ";  // Space-separated
    }

    // Close the file
    out_file.close();

    std::cout << "Data written to ./data/radial_stds.txt" << std::endl;
}

