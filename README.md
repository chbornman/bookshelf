# Bookshelf Management System

A simple C project to manage a collection of books.

## Building and Running

```sh
# Build the program
./build.sh

# Run the program
./bookshelf
```

## Features

- Track books with attributes like title, author, genre, cover type, condition, and word count
- Store books in a library collection
- Find books by title
- Display books and library contents

## Project Structure

- `book.h/c`: Book structure and related functions
- `library.h/c`: Library structure and management functions
- `main.c`: Main program entry point
- `build.sh`: Build script for compiling the program

## Memory Management Features

### Implemented:
- ✅ **Resizable Library Collection**: Dynamic memory allocation with `malloc`/`realloc` instead of fixed-size arrays

### Planned for future implementation:
1. **Dynamic String Handling**: Replace fixed-size character arrays with dynamically allocated strings for book titles and authors
2. **Book Notes System**: Add variable-length notes to books using dynamic memory allocation
3. **Custom Memory Pool**: Create a memory pool for book allocations to learn about memory fragmentation
4. **Memory Debugging Tools**: Add tracking for allocations/deallocations to detect memory leaks
5. **Book Lending System**: Implement reference counting for shared book resources
6. **Search Indexing Structure**: Build a dynamically allocated index to optimize book searching