#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "library.h"

// Initialize library with dynamic allocation
void initialize_library(Library* library) {
    library->count = 0;
    library->capacity = INITIAL_CAPACITY;
    library->books = (Book*)malloc(sizeof(Book) * library->capacity);
    
    if (!library->books) {
        fprintf(stderr, "Memory allocation failed during library initialization\n");
        library->capacity = 0;
    }
}

// Resize the library capacity
int resize_library(Library* library, int new_capacity) {
    if (new_capacity < library->count) {
        return 0; // Can't resize smaller than current count
    }
    
    Book* new_books = (Book*)realloc(library->books, sizeof(Book) * new_capacity);
    
    if (!new_books) {
        fprintf(stderr, "Memory reallocation failed during library resize\n");
        return 0;
    }
    
    library->books = new_books;
    library->capacity = new_capacity;
    printf("Library resized to capacity: %d\n", new_capacity);
    return 1;
}

// Add a book to the library
void add_book(Library* library, const Book* book) {
    // Check if we need to resize
    if (library->count >= library->capacity) {
        int new_capacity = library->capacity * GROWTH_FACTOR;
        if (!resize_library(library, new_capacity)) {
            printf("Failed to expand library capacity. Cannot add more books.\n");
            return;
        }
    }
    
    // Now we have space to add the book
    library->books[library->count] = *book;
    library->count++;
    printf("Added book: %s\n", book->title);
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

// Delete a book by title
int delete_book_by_title(Library* library, const char* title) {
    for (int i = 0; i < library->count; i++) {
        if (strcmp(library->books[i].title, title) == 0) {
            // Move the last book to this position (if it's not already the last)
            if (i < library->count - 1) {
                library->books[i] = library->books[library->count - 1];
            }
            library->count--;
            return 1; // Successful deletion
        }
    }
    return 0; // Book not found
}

// Free any allocated resources
void free_library(Library* library) {
    if (library->books) {
        free(library->books);
        library->books = NULL;
    }
    library->count = 0;
    library->capacity = 0;
}

// Helper function to escape CSV special characters in strings
static void escape_csv_field(const char* input, char* output, size_t output_size) {
    // Simple escaping: enclose in quotes if field contains delimiter, quotes, or newlines
    int needs_quotes = 0;
    
    for (const char* p = input; *p; p++) {
        if (*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
            needs_quotes = 1;
            break;
        }
    }
    
    size_t pos = 0;
    
    if (needs_quotes && pos < output_size - 1) {
        output[pos++] = '"';
    }
    
    for (const char* p = input; *p && pos < output_size - 2 - needs_quotes; p++) {
        // Double up quotes for escaping
        if (*p == '"') {
            output[pos++] = '"';
            if (pos < output_size - 2 - needs_quotes) {
                output[pos++] = '"';
            }
        } else {
            output[pos++] = *p;
        }
    }
    
    if (needs_quotes && pos < output_size - 1) {
        output[pos++] = '"';
    }
    
    output[pos] = '\0';
}

// Save library to CSV file
int save_library_to_csv(const Library* library, const char* filename) {
    // Skip if library is empty
    if (library->count == 0) {
        printf("Library is empty. Nothing to save.\n");
        return 1;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return 0;
    }
    
    // Write CSV header
    fprintf(file, "title,author,genre,cover_type,condition,word_count,year_published\n");
    
    char escaped_title[200];
    char escaped_author[200];
    
    // Write each book as a CSV row
    for (int i = 0; i < library->count; i++) {
        const Book* book = &library->books[i];
        
        escape_csv_field(book->title, escaped_title, sizeof(escaped_title));
        escape_csv_field(book->author, escaped_author, sizeof(escaped_author));
        
        fprintf(file, "%s,%s,%d,%d,%d,%d,%d\n",
                escaped_title,
                escaped_author,
                book->genre,
                book->cover_type,
                book->condition,
                book->word_count,
                book->year_published);
    }
    
    fclose(file);
    printf("Library saved to %s\n", filename);
    return 1;
}

// Parse a CSV line into tokens
static int parse_csv_line(char* line, char** tokens, int max_tokens) {
    int count = 0;
    int in_quotes = 0;
    char* p = line;
    tokens[count++] = p;
    
    while (*p && count < max_tokens) {
        if (*p == '"') {
            in_quotes = !in_quotes;
        } else if (*p == ',' && !in_quotes) {
            *p = '\0';
            tokens[count++] = p + 1;
        }
        p++;
    }
    
    // Remove quotes from quoted fields
    for (int i = 0; i < count; i++) {
        char* field = tokens[i];
        size_t len = strlen(field);
        
        if (len >= 2 && field[0] == '"' && field[len-1] == '"') {
            // Shift left to remove opening quote
            memmove(field, field + 1, len - 1);
            // Replace closing quote with null terminator
            field[len - 2] = '\0';
            
            // Handle doubled quotes inside field
            char* src = field;
            char* dst = field;
            while (*src) {
                if (*src == '"' && *(src + 1) == '"') {
                    *dst++ = '"';
                    src += 2;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';
        }
    }
    
    return count;
}

// Load library from CSV file
int load_library_from_csv(Library* library, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        // It's not an error if the file doesn't exist yet
        FILE* new_file = fopen(filename, "w");
        if (new_file) {
            // Create an empty file with header
            fprintf(new_file, "title,author,genre,cover_type,condition,word_count,year_published\n");
            fclose(new_file);
        }
        return 0;
    }
    
    char line[1024];
    int line_num = 0;
    
    // Note: We don't initialize library here anymore as it should be initialized before calling this function
    // The library is already initialized in main()
    
    // Read and parse each line
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            // Also handle \r\n
            if (len > 1 && line[len-2] == '\r') {
                line[len-2] = '\0';
            }
        }
        
        // Skip header row
        if (line_num == 1) {
            continue;
        }
        
        // Parse CSV line
        char* tokens[7];
        int token_count = parse_csv_line(line, tokens, 7);
        
        if (token_count != 7) {
            printf("Warning: Line %d contains %d fields (expected 7), skipping\n", 
                   line_num, token_count);
            continue;
        }
        
        // Create new book from parsed data
        Book book;
        strncpy(book.title, tokens[0], sizeof(book.title) - 1);
        book.title[sizeof(book.title) - 1] = '\0';
        
        strncpy(book.author, tokens[1], sizeof(book.author) - 1);
        book.author[sizeof(book.author) - 1] = '\0';
        
        book.genre = (Genre)atoi(tokens[2]);
        book.cover_type = (CoverType)atoi(tokens[3]);
        book.condition = (Condition)atoi(tokens[4]);
        book.word_count = atoi(tokens[5]);
        book.year_published = atoi(tokens[6]);
        
        // Add book to library - this will handle resizing if needed
        add_book(library, &book);
    }
    
    fclose(file);
    printf("Loaded %d books from %s\n", library->count, filename);
    return 1;
}

// Interactive CLI functions
void interactive_add_book(Library* library) {
    Book new_book;
    char buffer[256];
    int genre_choice, cover_choice, condition_choice;
    
    printf("\nADD NEW BOOK\n");
    printf("------------\n");
    
    // Get title
    printf("Enter title: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    strncpy(new_book.title, buffer, sizeof(new_book.title) - 1);
    new_book.title[sizeof(new_book.title) - 1] = '\0';
    
    // Get author
    printf("Enter author: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    strncpy(new_book.author, buffer, sizeof(new_book.author) - 1);
    new_book.author[sizeof(new_book.author) - 1] = '\0';
    
    // Get genre
    printf("Select genre:\n");
    printf("  0: Fiction\n");
    printf("  1: Non-Fiction\n");
    printf("  2: Mystery\n");
    printf("  3: Science Fiction\n");
    printf("  4: Fantasy\n");
    printf("  5: Biography\n");
    printf("  6: Reference\n");
    printf("Enter number (0-6): ");
    fgets(buffer, sizeof(buffer), stdin);
    genre_choice = atoi(buffer);
    if (genre_choice < 0 || genre_choice > 6) {
        printf("Invalid choice. Using Fiction as default.\n");
        genre_choice = 0;
    }
    new_book.genre = (Genre)genre_choice;
    
    // Get cover type
    printf("Select cover type:\n");
    printf("  0: Hardcover\n");
    printf("  1: Softcover\n");
    printf("  2: E-Book\n");
    printf("Enter number (0-2): ");
    fgets(buffer, sizeof(buffer), stdin);
    cover_choice = atoi(buffer);
    if (cover_choice < 0 || cover_choice > 2) {
        printf("Invalid choice. Using Hardcover as default.\n");
        cover_choice = 0;
    }
    new_book.cover_type = (CoverType)cover_choice;
    
    // Get condition
    printf("Select condition:\n");
    printf("  0: Excellent\n");
    printf("  1: Good\n");
    printf("  2: Fair\n");
    printf("  3: Poor\n");
    printf("Enter number (0-3): ");
    fgets(buffer, sizeof(buffer), stdin);
    condition_choice = atoi(buffer);
    if (condition_choice < 0 || condition_choice > 3) {
        printf("Invalid choice. Using Excellent as default.\n");
        condition_choice = 0;
    }
    new_book.condition = (Condition)condition_choice;
    
    // Get word count
    printf("Enter word count: ");
    fgets(buffer, sizeof(buffer), stdin);
    new_book.word_count = atoi(buffer);
    
    // Get year published
    printf("Enter year published: ");
    fgets(buffer, sizeof(buffer), stdin);
    new_book.year_published = atoi(buffer);
    
    // Add the book to the library
    add_book(library, &new_book);
    
    // Save the updated library
    save_library_to_csv(library, DEFAULT_CSV_FILE);
}

void interactive_lookup_book(Library* library) {
    char title[100];
    
    printf("\nLOOKUP BOOK\n");
    printf("-----------\n");
    printf("Enter book title: ");
    
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; // Remove newline
    
    Book* book = find_book_by_title(library, title);
    
    if (book) {
        printf("\nBook found:\n");
        print_book(book);
    } else {
        printf("\nBook not found: %s\n", title);
    }
}

void interactive_delete_book(Library* library) {
    char title[100];
    char confirm[10];
    
    printf("\nDELETE BOOK\n");
    printf("-----------\n");
    printf("Enter book title to delete: ");
    
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; // Remove newline
    
    Book* book = find_book_by_title(library, title);
    
    if (book) {
        printf("\nFound book to delete:\n");
        print_book(book);
        
        printf("\nAre you sure you want to delete this book? (yes/no): ");
        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = 0; // Remove newline
        
        // Convert to lowercase for comparison
        for (int i = 0; confirm[i]; i++) {
            confirm[i] = tolower(confirm[i]);
        }
        
        if (strcmp(confirm, "yes") == 0 || strcmp(confirm, "y") == 0) {
            if (delete_book_by_title(library, title)) {
                printf("Book deleted successfully.\n");
                save_library_to_csv(library, DEFAULT_CSV_FILE);
            } else {
                printf("Error deleting book.\n");
            }
        } else {
            printf("Deletion cancelled.\n");
        }
    } else {
        printf("\nBook not found: %s\n", title);
    }
}

void print_usage(const char* program_name) {
    printf("Bookshelf Management System\n");
    printf("Usage: %s [command]\n\n", program_name);
    printf("Commands:\n");
    printf("  add     - Add a new book (interactive)\n");
    printf("  lookup  - Look up a book by title\n");
    printf("  delete  - Delete a book by title\n");
    printf("  list    - List all books in the library\n");
    printf("  help    - Show this help message\n");
    printf("\nIf no command is given, the program will show all books.\n");
}