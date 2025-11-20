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

check_sha() {
    local source="$1"
    local dest="$2"
    [ ! -e "$dest" ] && return 1
    source_sha=$(sha256sum "$source" | awk '{print $1}')
    dest_sha=$(sha256sum "$dest" | awk '{print $1}')
    [ "$source_sha" != "$dest_sha" ]
}

ask_upgrade() {
    local path="$1"
    local name="$2"
    local source="$3"
    if [ ! -e "$path" ]; then
        echo "→ No $name found on system — installing fresh"
        return 0
    fi
    if ! check_sha "$source" "$path"; then
        echo "→ $name is already up to date — nothing to do"
        return 1
    fi
    echo "→ New version of $name available!"
    read -p "Upgrade? [Y/n] " -n 1 -r REPLY
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        echo "→ Skipping $name"
        return 1
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
if [ -n "$(ls -A . 2>/dev/null)" ]; then
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

if ask_upgrade "$INSTALL_DIR/$BINARY_NAME" "binary" "$BUILD_DIR/$BINARY_NAME"; then
    echo "Installing binary..."
    sudo cp "$BINARY_NAME" "$INSTALL_DIR/" && sudo chmod 755 "$INSTALL_DIR/$BINARY_NAME"
fi

if [ -f "$ICON_SOURCE" ]; then
    if ask_upgrade "$ICON_DEST" "application icon" "$ICON_SOURCE"; then
        echo "Installing icon..."
        sudo mkdir -p "$(dirname "$ICON_DEST")"
        sudo cp "$ICON_SOURCE" "$ICON_DEST"
        sudo gtk-update-icon-cache /usr/share/icons/hicolor -q 2>/dev/null || true
        echo "Icon installed"
    fi
else
    echo "Warning: Icon not found at $ICON_SOURCE"
fi

if [ -f "$DESKTOP_SOURCE" ]; then
    if ask_upgrade "$DESKTOP_DEST" ".desktop entry" "$DESKTOP_SOURCE"; then
        echo "Installing desktop entry..."
        sudo cp "$DESKTOP_SOURCE" "$DESKTOP_DEST"
        sudo chmod 644 "$DESKTOP_DEST"
        sudo update-desktop-database /usr/share/applications/ 2>/dev/null || true
    fi
else
    echo "Warning: .desktop file not found at $DESKTOP_SOURCE – skipping menu entry."
fi

echo
echo "========================================"
echo "Build and installation completed!"
echo "Run with: $BINARY_NAME"
echo "or find 'FFmpeg Converter' in your menu."
echo "========================================"
