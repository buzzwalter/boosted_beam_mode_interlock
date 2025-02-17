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
    && apt install -y cmake

# Verify CMake version
RUN cmake --version

# Install Halide via pip
RUN pip install halide --pre --extra-index-url https://test.pypi.org/simple

# Set working directory for the project
WORKDIR /workspace

# Default command: Open bash
CMD ["/bin/bash"]
