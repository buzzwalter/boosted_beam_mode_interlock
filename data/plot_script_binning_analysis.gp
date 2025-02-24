# Set the output format to PNG
set terminal png
set output 'binning_analysis_plot.png'

# Set the plot title and axis labels
set title "Binning Analysis"
set xlabel "Bin Number"
set ylabel "Standard Deviation of Time-Series"

# Plot the data
plot 'binning_results.txt' using 0:1 with linespoints title ""

