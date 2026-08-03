FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Enable aarch64 multiarch
RUN dpkg --add-architecture arm64

# Update and install cross-compilation tools + Qt5 arm64 + all Chiaki deps
RUN apt-get update && apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    git \
    protobuf-compiler \
    # Chiaki deps for arm64
    libssl-dev:arm64 \
    libopus-dev:arm64 \
    libavcodec-dev:arm64 \
    libavformat-dev:arm64 \
    libavutil-dev:arm64 \
    libswscale-dev:arm64 \
    libswresample-dev:arm64 \
    libsdl2-dev:arm64 \
    libasound2-dev:arm64 \
    libpulse-dev:arm64 \
    libevdev-dev:arm64 \
    libudev-dev:arm64 \
    # Qt5 for arm64
    qtbase5-dev:arm64 \
    qttools5-dev:arm64 \
    libqt5svg5-dev:arm64 \
    qtmultimedia5-dev:arm64 \
    libqt5opengl5-dev:arm64 \
    # Host Qt tools (lupdate, moc, rcc, uic need to run on host)
    qtbase5-dev \
    qttools5-dev \
    python3-pip \
    && pip3 install protobuf grpcio-tools \
    && rm -rf /var/lib/apt/lists/*

# Set up pkg-config for arm64 cross-compilation
ENV PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
ENV PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig

WORKDIR /build
