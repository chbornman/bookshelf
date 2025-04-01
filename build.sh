#!/bin/bash

# Build script for Bookshelf Management System

# Ensure script fails if any command fails
set -e

echo "Building Bookshelf Management System..."

# Create build directory if it doesn't exist
mkdir -p build

# Check for libcurl
echo "Checking for libcurl..."
if pkg-config --exists libcurl; then
    CURL_CFLAGS=$(pkg-config --cflags libcurl)
    CURL_LIBS=$(pkg-config --libs libcurl)
    echo "Found libcurl."
else
    echo "Warning: libcurl not found with pkg-config. Assuming default flags."
    CURL_LIBS="-lcurl"
fi

# Compile all source files separately
echo "Compiling book.c..."
clang -g -Wall -Wextra -std=c99 $CURL_CFLAGS -c book.c -o build/book.o

echo "Compiling cJSON.c..."
clang -g -Wall -Wextra -std=c99 $CURL_CFLAGS -c cJSON.c -o build/cJSON.o

echo "Compiling library.c..."
clang -g -Wall -Wextra -std=c99 $CURL_CFLAGS -c library.c -o build/library.o

echo "Compiling main.c..."
clang -g -Wall -Wextra -std=c99 $CURL_CFLAGS -c main.c -o build/main.o

# Link all object files together
echo "Linking with libcurl..."
clang build/book.o build/cJSON.o build/library.o build/main.o -o bookshelf $CURL_LIBS

# Make the output executable
chmod +x bookshelf

echo "Build successful! Run './bookshelf help' for usage instructions."
echo "Run './bookshelf fetch-metadata' to retrieve book information from Open Library API."