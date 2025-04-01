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