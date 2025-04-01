#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "book.h"
#include "library.h"

int main() {
    printf("Bookshelf Management System\n\n");
    
    Library library;
    initialize_library(&library);
    
    // Add some sample books
    Book book1 = {
        .title = "The Great Gatsby",
        .author = "F. Scott Fitzgerald",
        .genre = FICTION,
        .cover_type = HARDCOVER,
        .condition = GOOD,
        .word_count = 47094,
        .year_published = 1925
    };
    
    Book book2 = {
        .title = "Clean Code",
        .author = "Robert C. Martin",
        .genre = NONFICTION,
        .cover_type = SOFTCOVER,
        .condition = EXCELLENT,
        .word_count = 76000,
        .year_published = 2008
    };
    
    add_book(&library, &book1);
    add_book(&library, &book2);
    
    print_library(&library);
    
    Book* found = find_book_by_title(&library, "Clean Code");
    if (found) {
        printf("\nFound book: %s by %s\n", found->title, found->author);
    }
    
    free_library(&library);
    
    return 0;
}