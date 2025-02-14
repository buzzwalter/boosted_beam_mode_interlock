# Use Ubuntu as the base image
FROM ubuntu:22.04

# Set noninteractive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies
RUN apt update && apt install -y \
    build-essential \
    cmake \
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

# Set working directory
WORKDIR /usr/local

# Download and build Halide
RUN git clone --recursive https://github.com/halide/Halide.git && \
    cd Halide && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -G Ninja .. && \
    ninja && ninja install

# Set environment variables for Halide
ENV PATH="/usr/local/Halide/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/local/Halide/lib:${LD_LIBRARY_PATH}"

# Set working directory for the project
WORKDIR /workspace

# Default command: Open bash
CMD ["/bin/bash"]
