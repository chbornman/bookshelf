#!/bin/bash

# Build script for Bookshelf Management System

# Ensure script fails if any command fails
set -e

echo "Building Bookshelf Management System..."

# Compile using clang with the standard C99 flag, all warnings enabled, and debug symbols
clang -g -Wall -Wextra -std=c99 main.c -o bookshelf

# Make the output executable
chmod +x bookshelf

echo "Build successful! Run './bookshelf help' for usage instructions."