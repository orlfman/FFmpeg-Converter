#!/bin/bash
PROJECT_DIR="/mnt/storage2/Source Projects/ffmpeg_converter_qt"
BACKUP_DIR="/mnt/storage2/Source Projects/ffmpeg_converter_qt-back"
BUILD_DIR="$PROJECT_DIR/build"
BINARY_NAME="ffmpeg_converter_qt"
INSTALL_DIR="/usr/local/bin"
# Function to check for dependencies
check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed. Please install it."
        exit 1
    fi
}
# Create backup
echo "Creating backup..."
if [ ! -d "$PROJECT_DIR" ]; then
    echo "Error: Source directory $PROJECT_DIR does not exist."
    exit 1
fi
# If backup directory exists, rename it with a timestamp
if [ -d "$BACKUP_DIR" ]; then
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    BACKUP_ARCHIVE="$BACKUP_DIR-$TIMESTAMP"
    echo "Previous backup found, renaming to $BACKUP_ARCHIVE..."
    mv "$BACKUP_DIR" "$BACKUP_ARCHIVE" || { echo "Error: Failed to rename previous backup"; exit 1; }
fi
# Copy project directory to backup
cp -r "$PROJECT_DIR" "$BACKUP_DIR" || { echo "Error: Failed to create backup at $BACKUP_DIR"; exit 1; }
echo "Backup created successfully at $BACKUP_DIR"
# Check required dependencies
echo "Checking dependencies..."
check_dependency cmake
check_dependency make
check_dependency g++
check_dependency ffmpeg
check_dependency qmake6
if ! qmake6 --version | grep -q "Qt version 6"; then
    echo "Error: Qt 6 is required. Please install Qt 6 libraries."
    exit 1
fi
# Create and enter build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { echo "Error: Cannot change to build directory"; exit 1; }
# Clean build directory
if [ -n "$(ls -A)" ]; then
    echo "Cleaning build directory..."
    rm -rf *
fi
# Run cmake
echo "Running cmake..."
sleep 2s
cmake .. || { echo "Error: cmake failed"; exit 1; }
# Run build
echo "Running build..."
cmake --build . || { echo "Error: build failed"; exit 1; }
# Check for binary
if [ ! -f "$BINARY_NAME" ]; then
    echo "Error: Binary '$BINARY_NAME' not found in $BUILD_DIR"
    exit 1
fi
# Check if binary already exists in /usr/local/bin and prompt for overwrite
if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
    echo "Warning: '$BINARY_NAME' already exists in $INSTALL_DIR."
    read -p "Do you want to overwrite it? [y/N] " REPLY
    if [ "$REPLY" != "y" ] && [ "$REPLY" != "Y" ]; then
        echo "Installation aborted."
        exit 0
    fi
fi
# Install binary
echo "Installing binary to $INSTALL_DIR..."
sudo cp "$BINARY_NAME" "$INSTALL_DIR/" || { echo "Error: Failed to copy binary to $INSTALL_DIR"; exit 1; }
echo "Build and installation completed successfully!"
