#ifndef BOOK_H
#define BOOK_H

// Enums for book attributes
typedef enum {
    HARDCOVER,
    SOFTCOVER,
    EBOOK
} CoverType;

typedef enum {
    FICTION,
    NONFICTION,
    MYSTERY,
    SCIFI,
    FANTASY,
    BIOGRAPHY,
    REFERENCE
} Genre;

typedef enum {
    EXCELLENT,
    GOOD,
    FAIR,
    POOR
} Condition;

// Book structure
typedef struct {
    char title[100];
    char author[100];
    char isbn[20];      // Standard ISBN is 13 digits, plus hyphens
    char genre[50];     // Genre as a string instead of enum
    CoverType cover_type;
    Condition condition;
    int word_count;
    int year_published;
    int metadata_retrieved;  // Boolean flag to track if metadata was already fetched
} Book;

// Function declarations
void print_book(const Book* book);
const char* get_cover_type_string(CoverType cover_type);
const char* get_condition_string(Condition condition);

#endif // BOOK_H