#!/bin/sh
rm -rf build
meson setup build
meson compile -C build

cp -r res build
