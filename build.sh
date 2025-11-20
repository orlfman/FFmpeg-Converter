#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_DIR/build"
BINARY_NAME="ffmpeg_converter_qt"
INSTALL_DIR="/usr/local/bin"
DESKTOP_SOURCE="$PROJECT_DIR/FFmpegConverter.desktop"
DESKTOP_DEST="/usr/share/applications/FFmpegConverter.desktop"
ICON_SOURCE="$PROJECT_DIR/ffmpeg-converter-qt.png"
ICON_DEST="/usr/share/icons/hicolor/512x512/apps/ffmpeg-converter-qt.png"

check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed. Please install it and try again."
        exit 1
    fi
}

ask_overwrite() {
    local path="$1"
    local name="$2"
    if [ -e "$path" ]; then
        echo "Warning: $name already exists at $path"
        read -p "Overwrite? [y/N] " -n 1 -r REPLY
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            return 0
        else
            echo "→ Skipping $name"
            return 1
        fi
    else
        return 0
    fi
}

echo "Detected project directory: $PROJECT_DIR"
echo "Checking required tools..."
check_dependency cmake
check_dependency make
check_dependency g++
check_dependency ffmpeg
check_dependency qmake6

if ! qmake6 --version | grep -q "Qt version 6"; then
    echo "Error: This project requires Qt 6. Make sure Qt6 development packages are installed."
    exit 1
fi

echo "Preparing build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

if [ -n "$(ls -A .)" ]; then
    echo "Cleaning previous build files..."
    rm -rf ./*
fi

echo "Configuring with cmake..."
cmake .. || { echo "cmake failed"; exit 1; }

echo "Building the project..."
cmake --build . -- -j$(nproc) || { echo "Build failed"; exit 1; }

if [ ! -f "$BINARY_NAME" ]; then
    echo "Error: Binary '$BINARY_NAME' was not created."
    exit 1
fi

echo "Build successful!"

if ask_overwrite "$INSTALL_DIR/$BINARY_NAME" "binary ($BINARY_NAME)"; then
    echo "Installing binary to $INSTALL_DIR..."
    sudo cp "$BINARY_NAME" "$INSTALL_DIR/" || { echo "Failed to install binary"; exit 1; }
    sudo chmod 755 "$INSTALL_DIR/$BINARY_NAME"
fi

if [ -f "$ICON_SOURCE" ]; then
    if ask_overwrite "$ICON_DEST" "application icon"; then
        echo "Installing icon to hicolor theme..."
        sudo mkdir -p "/usr/share/icons/hicolor/512x512/apps"
        sudo cp "$ICON_SOURCE" "$ICON_DEST"
        sudo gtk-update-icon-cache /usr/share/icons/hicolor -q 2>/dev/null || true
        sudo xdg-icon-resource forceupdate 2>/dev/null || true
        echo "Icon installed and cache updated"
    fi
else
    echo "Warning: Icon not found at $ICON_SOURCE (looked in $PROJECT_DIR)"
fi

if [ -f "$DESKTOP_SOURCE" ]; then
    if ask_overwrite "$DESKTOP_DEST" ".desktop entry"; then
        echo "Installing desktop entry..."
        sudo cp "$DESKTOP_SOURCE" "$DESKTOP_DEST" || { echo "Failed to install .desktop file"; exit 1; }
        sudo chmod 644 "$DESKTOP_DEST"
        sudo update-desktop-database /usr/share/applications/ 2>/dev/null || true
    fi
else
    echo "Warning: .desktop file not found at $DESKTOP_SOURCE – skipping menu entry."
fi

echo
echo "========================================"
echo "Build and installation completed!"
echo "You can now run the app with: $BINARY_NAME"
echo "or find 'FFmpeg Converter' in your application menu."
echo "========================================"
