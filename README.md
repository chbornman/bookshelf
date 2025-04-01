# Bookshelf Management System

A simple C project to manage a collection of books.

## Building and Running

### Regular Build
```sh
make
./bookshelf
```

### Unity Build
```sh
make unity
./bookshelf_unity
```

## Features

- Track books with attributes like title, author, genre, cover type, condition, and word count
- Store books in a library collection
- Find books by title
- Display books and library contents

## Project Structure

- `book.h/c`: Book structure and related functions
- `library.h/c`: Library structure and management functions
- `bookshelf.c`: Main program
- `unity_build.c`: Unity build file that includes all source files

## Unity Build

This project demonstrates a "Unity Build" approach, where all source files are included in a single compilation unit to potentially improve build times for larger projects.

## Future Memory Management Features

The following features are planned for future implementation to learn and practice C memory management:

1. **Dynamic String Handling**: Replace fixed-size character arrays with dynamically allocated strings for book titles and authors
2. **Resizable Library Collection**: Implement dynamic memory allocation with `malloc`/`realloc` instead of fixed-size arrays
3. **Book Notes System**: Add variable-length notes to books using dynamic memory allocation
4. **Custom Memory Pool**: Create a memory pool for book allocations to learn about memory fragmentation
5. **Memory Debugging Tools**: Add tracking for allocations/deallocations to detect memory leaks
6. **Book Lending System**: Implement reference counting for shared book resources
7. **Search Indexing Structure**: Build a dynamically allocated index to optimize book searching