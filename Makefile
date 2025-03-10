CXX = g++
CXXFLAGS = -std=c++11 -Wall -fopenmp -c
OPENCV_FLAGS = $(shell pkg-config --cflags opencv4)
PYLON_INCLUDE = -I/opt/pylon/include
INCLUDES = $(OPENCV_FLAGS) $(PYLON_INCLUDE)

# Order of libraries is important for resolving dependencies
LDFLAGS = -fopenmp
OPENCV_LIBS = $(shell pkg-config --libs opencv4)
PYLON_LIBS = -L/opt/pylon/lib -lpylonbase -lpylonutility -lGenApi_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon
FFTW_LIBS = -lfftw3
LIBS = $(FFTW_LIBS) $(PYLON_LIBS) $(OPENCV_LIBS)

TARGET = spectrum_analyzer
OBJECTS = main.o fourier_spectrum.o basler_camera.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

main.o: main.cpp fourier_spectrum.hpp basler_camera.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) main.cpp

fourier_spectrum.o: fourier_spectrum.cpp fourier_spectrum.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) fourier_spectrum.cpp

basler_camera.o: basler_camera.cpp basler_camera.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) basler_camera.cpp

clean:
	rm -f $(TARGET) $(OBJECTS)