# Set the output format to PNG
set terminal png
set output 'radial_stds_plot.png'

# Set the plot title and axis labels
set title "Radial Standard Deviations"
set xlabel "Frame Number"
set ylabel "Standard Deviation"

# Plot the data
plot 'radial_stds.txt' using 0:2 with linespoints title ""

