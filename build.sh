#!/bin/bash

# Build script for ABLS Agent GPIOD
# This script builds the project in the 'build' directory using CMake

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

if [ "${1:-}" = "clean" ]; then
    rm -rf "$BUILD_DIR"
fi

echo "Building ABLS Agent GPIOD..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Number of processors: $(nproc)"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

echo "Running CMake..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"

echo "Building project..."
cmake --build "$BUILD_DIR" -- -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Built artifacts are in: $BUILD_DIR"
echo "Install with ./install.sh"
