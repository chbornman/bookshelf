#include <stdio.h>
#include "book.h"

// Convert enums to strings for display
const char* get_genre_string(Genre genre) {
    switch (genre) {
        case FICTION: return "Fiction";
        case NONFICTION: return "Non-Fiction";
        case MYSTERY: return "Mystery";
        case SCIFI: return "Science Fiction";
        case FANTASY: return "Fantasy";
        case BIOGRAPHY: return "Biography";
        case REFERENCE: return "Reference";
        default: return "Unknown";
    }
}

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
    printf("%s by %s\n", book->title, book->author);
    printf("  Genre: %s\n", get_genre_string(book->genre));
    printf("  Cover: %s\n", get_cover_type_string(book->cover_type));
    printf("  Condition: %s\n", get_condition_string(book->condition));
    printf("  Word Count: %d\n", book->word_count);
    printf("  Published: %d\n", book->year_published);
}