# Use Ubuntu 22.04 as base image
FROM ubuntu:22.04

# Set noninteractive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt update && apt install -y \
    build-essential \
    clang \
    lld \
    libc++-dev \
    libc++abi-dev \
    libtinfo-dev \
    libpng-dev \
    libjpeg-dev \
    libllvm15 \
    libclang-15-dev \
    python3 \
    python3-pip \
    ninja-build \
    git \
    wget \
    unzip \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Remove outdated CMake (if installed)
RUN apt remove -y cmake || true

# Install CMake 3.28+ from Kitware repository
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt update \
    && apt install -y cmake \ 
    libopencv-dev \
    libfftw3-dev \ 
    nano \
    llvm-14 \ 
    llvm-14-dev \ 
    clang-14

# Verify CMake version
RUN cmake --version

# Install and link halide via 
RUN wget https://github.com/halide/Halide/releases/download/v19.0.0/Halide-19.0.0-arm-64-linux-5f17d6f8a35e7d374ef2e7e6b2d90061c0530333.tar.gz \
    && tar -xvzf Halide-19.0.0-arm-64-linux-5f17d6f8a35e7d374ef2e7e6b2d90061c0530333.tar.gz \
    &&  mv Halide-19.0.0-arm-64-linux /usr/local/halide \
    && ln -s /usr/local/halide/lib/libHalide.so /usr/local/lib/libHalide.so \ 
    && rm Halide-19.0.0-arm-64-linux-5f17d6f8a35e7d374ef2e7e6b2d90061c0530333.tar.gz

# Set ENVs
ENV export LLVM_CONFIG=/usr/bin/llvm-config-14
ENV export PATH=/usr/local/halide/bin:$PATH
ENV export CPLUS_INCLUDE_PATH=/usr/local/halide/include:$CPLUS_INCLUDE_PATH
ENV export LD_LIBRARY_PATH=/usr/local/halide/lib:$LD_LIBRARY_PATH

# If you want debug output, run the following as well
# RUN export HL_DEBUG = 1 && export HL_DEBUG_CODEGEN = 1

# Set working directory for the project
WORKDIR /workspace

# Default command: Open bash
CMD ["/bin/bash"]
