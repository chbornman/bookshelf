#ifndef LIBRARY_H
#define LIBRARY_H

#include "book.h"

#define INITIAL_CAPACITY 10
#define GROWTH_FACTOR 2
#define CSV_DELIMITER ","
#define DEFAULT_CSV_FILE "bookshelf.csv"

// Library structure to hold books with dynamic allocation
typedef struct {
    Book* books;       // Dynamically allocated array of books
    int count;         // Number of books currently in the library
    int capacity;      // Total capacity of the allocated array
} Library;

// Function declarations
void initialize_library(Library* library);
void add_book(Library* library, const Book* book);
void print_library(const Library* library);
Book* find_book_by_title(Library* library, const char* title);
int delete_book_by_title(Library* library, const char* title);
void free_library(Library* library);

// Memory management functions
int resize_library(Library* library, int new_capacity);

// CSV functions
int save_library_to_csv(const Library* library, const char* filename);
int load_library_from_csv(Library* library, const char* filename);

// Interactive CLI functions
void interactive_add_book(Library* library);
void interactive_lookup_book(Library* library);
void interactive_delete_book(Library* library);
void print_usage(const char* program_name);

#endif // LIBRARY_H
