#!/bin/bash

# Build script for Bookshelf Management System

# Ensure script fails if any command fails
set -e

echo "Building Bookshelf Management System..."

# Create build directory if it doesn't exist
mkdir -p build

# Compile all source files separately
echo "Compiling book.c..."
clang -g -Wall -Wextra -std=c99 -c book.c -o build/book.o

echo "Compiling library.c..."
clang -g -Wall -Wextra -std=c99 -c library.c -o build/library.o

echo "Compiling main.c..."
clang -g -Wall -Wextra -std=c99 -c main.c -o build/main.o

# Link all object files together
echo "Linking..."
clang build/book.o build/library.o build/main.o -o bookshelf

# Make the output executable
chmod +x bookshelf

echo "Build successful! Run './bookshelf help' for usage instructions."