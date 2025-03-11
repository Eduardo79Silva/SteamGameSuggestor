#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

echo "Starting Linux CI build for SteamSuggestor"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Setup build directory
BUILD_DIR="$PROJECT_ROOT/build"
#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

echo "Starting Linux CI build for SteamSuggestor"

# Determine script location and project root
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
# If running from scripts directory, go one level up to find project root
if [[ "$SCRIPT_DIR" == */scripts ]]; then
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
else
    PROJECT_ROOT="$SCRIPT_DIR"
fi

echo "Project root: $PROJECT_ROOT"

# Setup build directory
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# Install dependencies
echo "Installing dependencies..."
if command -v apt-get &>/dev/null; then
    # Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libcurl4-openssl-dev libgtest-dev
elif command -v dnf &>/dev/null; then
    # Fedora/RHEL
    sudo dnf install -y gcc gcc-c++ make cmake libcurl-devel gtest-devel
elif command -v pacman &>/dev/null; then
    # Arch Linux
    sudo pacman -Sy --noconfirm base-devel cmake curl gtest
elif command -v zypper &>/dev/null; then
    # openSUSE
    sudo zypper install -y gcc gcc-c++ make cmake libcurl-devel gtest-devel
else
    echo "Unsupported package manager. Please install the following manually:"
    echo "  - C++ compiler (g++ or clang++)"
    echo "  - CMake"
    echo "  - libcurl development headers"
    echo "  - Google Test"
    exit 1
fi

# If GTest is not available as a package, build it from source
if [ ! -f "/usr/include/gtest/gtest.h" ] && [ ! -f "/usr/local/include/gtest/gtest.h" ]; then
    echo "Building Google Test from source..."
    if [ ! -d "$PROJECT_ROOT/lib/gtest" ]; then
        mkdir -p "$PROJECT_ROOT/lib"
        git clone https://github.com/google/googletest.git "$PROJECT_ROOT/lib/gtest"
    fi
fi

# Build project
echo "Building SteamSuggestor..."
cd "$BUILD_DIR"
cmake "$PROJECT_ROOT"
make -j$(nproc)

# Run tests
echo "Running tests..."
if [ -f "$BUILD_DIR/run_tests" ]; then
    "$BUILD_DIR/run_tests"
else
    echo "Warning: Test executable not found. Skipping tests."
fi

# Create distribution package
echo "Creating distribution package..."
make install DESTDIR="$BUILD_DIR/package"
cd "$BUILD_DIR/package"
tar -czvf "$BUILD_DIR/SteamSuggestor-Linux.tar.gz" .

echo "Build completed successfully!"
echo "Binary located at: $BUILD_DIR/SteamSuggestor.exe"
echo "Package located at: $BUILD_DIR/SteamSuggestor-Linux.tar.gz"
