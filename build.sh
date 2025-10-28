#!/bin/bash

# Replit3DS Build Script
# This script checks for devkitPro toolchain and builds the 3DS application

echo "========================================"
echo "  Replit3DS - Nintendo 3DS IDE Builder"
echo "========================================"
echo ""

# Check if devkitPro is installed
if [ -z "$DEVKITPRO" ]; then
    echo "❌ devkitPro not detected in this environment"
    echo ""
    echo "This Nintendo 3DS application requires the devkitPro"
    echo "toolchain to compile ARM code for the 3DS console."
    echo ""
    echo "📚 To build this project, you have two options:"
    echo ""
    echo "Option 1: Build on your local machine"
    echo "  1. Clone this repository to your local machine"
    echo "  2. Install devkitPro from: https://devkitpro.org"
    echo "  3. Run: sudo dkp-pacman -S 3ds-dev"
    echo "  4. Run: make"
    echo ""
    echo "Option 2: Use GitHub Actions / CI/CD"
    echo "  - Set up automated builds with devkitPro Docker images"
    echo "  - Example: devkitpro/devkitarm:latest"
    echo ""
    echo "📦 Build Output:"
    echo "  - Replit3DS.3dsx (Homebrew Launcher format)"
    echo "  - Replit3DS.elf (Executable)"
    echo ""
    echo "🎮 To run on Nintendo 3DS:"
    echo "  1. Copy Replit3DS.3dsx to /3ds/ on SD card"
    echo "  2. Launch via Homebrew Launcher"
    echo ""
    exit 1
fi

# devkitPro is installed, proceed with build
echo "✓ devkitPro found at: $DEVKITPRO"
echo "✓ devkitARM found at: $DEVKITARM"
echo ""

# Check for required libraries
echo "Checking dependencies..."

if [ ! -d "$DEVKITPRO/libctru" ]; then
    echo "❌ libctru not found"
    echo "   Install with: sudo dkp-pacman -S libctru"
    exit 1
fi
echo "✓ libctru found"

if [ ! -d "$DEVKITPRO/portlibs/3ds" ]; then
    echo "⚠  3DS portlibs not found (optional)"
else
    echo "✓ 3DS portlibs found"
fi

echo ""
echo "Building project..."
echo ""

# Run make
make clean
if make; then
    echo ""
    echo "========================================"
    echo "  ✅ Build successful!"
    echo "========================================"
    echo ""
    echo "Output files:"
    ls -lh Replit3DS.3dsx Replit3DS.elf 2>/dev/null || echo "Build artifacts not found"
    echo ""
    echo "Next steps:"
    echo "  1. Copy Replit3DS.3dsx to /3ds/ on your SD card"
    echo "  2. Insert SD card into Nintendo 3DS"
    echo "  3. Launch Homebrew Launcher"
    echo "  4. Select Replit3DS from the app list"
    echo ""
else
    echo ""
    echo "========================================"
    echo "  ❌ Build failed"
    echo "========================================"
    echo ""
    echo "Check the error messages above for details."
    exit 1
fi
