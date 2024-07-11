#!/bin/sh

# Exit immediately if a command exits with a non-zero status.
set -e

# Remove the build directory if it exists.
rm -rf build

# Setup the build directory with Meson.
meson setup build

# Compile the project in the build directory.
meson compile -C build
