#include <stdio.h>
#include <string.h>
#include "library.h"

// Initialize library
void initialize_library(Library* library) {
    library->count = 0;
}

// Add a book to the library
void add_book(Library* library, const Book* book) {
    if (library->count < MAX_BOOKS) {
        library->books[library->count] = *book;
        library->count++;
        printf("Added book: %s\n", book->title);
    } else {
        printf("Library is full. Cannot add more books.\n");
    }
}

// Print all books in the library
void print_library(const Library* library) {
    printf("\nLibrary Contents (%d books):\n", library->count);
    printf("------------------------\n");
    
    for (int i = 0; i < library->count; i++) {
        printf("Book %d:\n", i + 1);
        print_book(&library->books[i]);
        printf("\n");
    }
}

// Find a book by title
Book* find_book_by_title(Library* library, const char* title) {
    for (int i = 0; i < library->count; i++) {
        if (strcmp(library->books[i].title, title) == 0) {
            return &library->books[i];
        }
    }
    return NULL;
}

// Free any allocated resources
void free_library(Library* library) {
    // Currently nothing to free, but this function can be expanded later
    // if dynamic memory allocation is used
    library->count = 0;
}