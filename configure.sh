#!/bin/sh

# Build system.
brew install meson

# SFML library dependencies.
brew install flac freetype libogg libvorbis

# GUI Platform backend dependency.
brew install sdl2

# Create the .mlbc directory in the user's home directory if it doesn't exist.
mkdir -p "$HOME/.mlbc"

# Copy the res directory to the .mlbc directory in the user's home directory.
cp -r res "$HOME/.mlbc"
