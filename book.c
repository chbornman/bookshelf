#include <stdio.h>
#include "book.h"

// Enum to string functions

const char* get_cover_type_string(CoverType cover_type) {
    switch (cover_type) {
        case HARDCOVER: return "Hardcover";
        case SOFTCOVER: return "Softcover";
        case EBOOK: return "E-Book";
        default: return "Unknown";
    }
}

const char* get_condition_string(Condition condition) {
    switch (condition) {
        case EXCELLENT: return "Excellent";
        case GOOD: return "Good";
        case FAIR: return "Fair";
        case POOR: return "Poor";
        default: return "Unknown";
    }
}

// Print book details
void print_book(const Book* book) {
    // For title and author, show "<empty>" if the field is empty
    printf("%s by %s\n", 
           book->title[0] != '\0' ? book->title : "<empty>", 
           book->author[0] != '\0' ? book->author : "<empty>");
    
    // For ISBN, only show if not empty
    if (book->isbn[0] != '\0') {
        printf("  ISBN: %s\n", book->isbn);
        if (book->metadata_retrieved) {
            printf("  Metadata: Already retrieved\n");
        }
    } else {
        printf("  ISBN: <empty>\n");
    }
    
    // For genre, show "<empty>" if the field is empty
    printf("  Genre: %s\n", book->genre[0] != '\0' ? book->genre : "<empty>");
    printf("  Cover: %s\n", get_cover_type_string(book->cover_type));
    printf("  Condition: %s\n", get_condition_string(book->condition));
    
    // For numeric fields, show "<empty>" if the value is 0 or negative
    if (book->word_count > 0) {
        printf("  Word Count: %d\n", book->word_count);
    } else {
        printf("  Word Count: <empty>\n");
    }
    
    if (book->year_published > 0) {
        printf("  Published: %d\n", book->year_published);
    } else {
        printf("  Published: <empty>\n");
    }
}