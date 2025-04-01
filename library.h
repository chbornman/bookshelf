#ifndef LIBRARY_H
#define LIBRARY_H

#include "book.h"

#define MAX_BOOKS 100

// Library structure to hold books
typedef struct {
    Book books[MAX_BOOKS];
    int count;
} Library;

// Function declarations
void initialize_library(Library* library);
void add_book(Library* library, const Book* book);
void print_library(const Library* library);
Book* find_book_by_title(Library* library, const char* title);
void free_library(Library* library);

#endif // LIBRARY_H