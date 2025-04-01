# Bookshelf Management System

A simple C project to manage a collection of books with barcode scanner support and Open Library API integration.

## Building and Running

```sh
# Build the program
./build.sh

# Show usage instructions
./bookshelf help
```

## Features

- Track books with attributes like title, author, ISBN, genre, cover type, condition, and word count
- Store books in a library collection with CSV persistence
- Find books by title
- Delete books from the collection
- Fetch book metadata from Open Library API using ISBN numbers
- Support for barcode scanner input (ISBN scanning)
- Smart metadata retrieval that avoids unnecessary API calls

## Usage

```bash
# Show help
./bookshelf help

# Add a new book (interactive mode)
./bookshelf add

# List all books in the library
./bookshelf list

# Look up a book by title
./bookshelf lookup

# Delete a book
./bookshelf delete

# Fetch metadata for books with ISBNs (only for books that haven't been fetched yet)
./bookshelf fetch-metadata

# Force fetch metadata for all books with ISBNs (even if already fetched)
./bookshelf fetch-metadata --force
```

## Barcode Scanner Integration

This system supports ISBN barcode scanners that work as keyboard emulators (most USB scanners). When adding books, you have the following workflows:

### Quick Scan Workflow
1. Run `./bookshelf add`
2. Choose option 2 for barcode scanning
3. Scan ISBN from your book
4. Choose option 2 to save with just the ISBN
5. Accept the prompt to fetch metadata automatically
6. Done! Book is added with complete details from Open Library

### Manual Entry Workflow
1. Run `./bookshelf add`
2. Choose option 1 for manual entry
3. Enter book details as prompted

### Batch Scanning Workflow
1. Run `./bookshelf add` and add multiple books with just ISBNs
2. Run `./bookshelf fetch-metadata` to retrieve details for all books at once

## Metadata Management

- The system tracks which books have already had metadata retrieved to avoid unnecessary API calls
- Books display a "Metadata: Already retrieved" indicator when their data has been fetched
- Use the `--force` flag with `fetch-metadata` to update all books regardless of previous retrieval status

## Requirements

- C compiler (gcc or clang)
- libcurl library for API requests
- Basic terminal environment
- Optional: USB barcode scanner that acts as a keyboard input device

## Project Structure

- `book.h/c`: Book structure and related functions
- `library.h/c`: Library structure and management functions (including API integration)
- `main.c`: Main program entry point
- `build.sh`: Build script for compiling the program
- `bookshelf.csv`: CSV storage file for your book collection
