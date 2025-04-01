#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <curl/curl.h>  // libcurl for HTTP requests
#include "library.h"

/* cJSON implementation */
#include "cJSON.h"

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
    fprintf(file, "title,author,isbn,genre,cover_type,condition,word_count,year_published,metadata_retrieved\n");
    
    char escaped_title[200];
    char escaped_author[200];
    char escaped_isbn[40];
    char escaped_genre[100];
    
    // Write each book as a CSV row
    for (int i = 0; i < library->count; i++) {
        const Book* book = &library->books[i];
        
        escape_csv_field(book->title, escaped_title, sizeof(escaped_title));
        escape_csv_field(book->author, escaped_author, sizeof(escaped_author));
        escape_csv_field(book->isbn, escaped_isbn, sizeof(escaped_isbn));
        escape_csv_field(book->genre, escaped_genre, sizeof(escaped_genre));
        
        fprintf(file, "%s,%s,%s,%s,%d,%d,%d,%d,%d\n",
                escaped_title,
                escaped_author,
                escaped_isbn,
                escaped_genre,
                book->cover_type,
                book->condition,
                book->word_count,
                book->year_published,
                book->metadata_retrieved);
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
            fprintf(new_file, "title,author,isbn,genre,cover_type,condition,word_count,year_published,metadata_retrieved\n");
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
        char* tokens[9];
        int token_count = parse_csv_line(line, tokens, 9);
        
        // Handle both old (8 column) and new (9 column) formats
        if (token_count != 8 && token_count != 9) {
            printf("Warning: Line %d contains %d fields (expected 8 or 9), skipping\n", 
                   line_num, token_count);
            continue;
        }
        
        // Create new book from parsed data
        Book book;
        memset(&book, 0, sizeof(Book)); // Initialize all fields to 0/NULL
        
        strncpy(book.title, tokens[0], sizeof(book.title) - 1);
        book.title[sizeof(book.title) - 1] = '\0';
        
        strncpy(book.author, tokens[1], sizeof(book.author) - 1);
        book.author[sizeof(book.author) - 1] = '\0';
        
        strncpy(book.isbn, tokens[2], sizeof(book.isbn) - 1);
        book.isbn[sizeof(book.isbn) - 1] = '\0';
        
        strncpy(book.genre, tokens[3], sizeof(book.genre) - 1);
        book.genre[sizeof(book.genre) - 1] = '\0';
        book.cover_type = (CoverType)atoi(tokens[4]);
        book.condition = (Condition)atoi(tokens[5]);
        book.word_count = atoi(tokens[6]);
        book.year_published = atoi(tokens[7]);
        
        // Handle metadata_retrieved flag (if present in the CSV)
        if (token_count == 9) {
            book.metadata_retrieved = atoi(tokens[8]);
        } else {
            book.metadata_retrieved = 0; // Default for backward compatibility
        }
        
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
    int cover_choice, condition_choice, input_method;
    
    printf("\nADD NEW BOOK\n");
    printf("------------\n");
    
    // Initialize the book with zeros
    memset(&new_book, 0, sizeof(Book));
    
    // Explicitly set metadata_retrieved to 0
    new_book.metadata_retrieved = 0;
    
    // Choose input method
    printf("How would you like to add this book?\n");
    printf("  1: Enter all information manually\n");
    printf("  2: Scan barcode for ISBN only (fetch details later)\n");
    printf("Enter choice (1-2): ");
    fgets(buffer, sizeof(buffer), stdin);
    input_method = atoi(buffer);
    
    if (input_method == 2) {
        // ISBN barcode scanning method
        printf("\nPlease scan the book's ISBN barcode now...\n");
        printf("[Waiting for scanner input...]\n");
        
        // Read ISBN from stdin (simulating barcode scanner input)
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        
        // Store the ISBN
        strncpy(new_book.isbn, buffer, sizeof(new_book.isbn) - 1);
        new_book.isbn[sizeof(new_book.isbn) - 1] = '\0';
        
        printf("ISBN scanned: %s\n", new_book.isbn);
        
        // Ask if user wants to add minimal required info or fetch later
        printf("\nWould you like to:\n");
        printf("  1: Add minimal required info now (title required)\n");
        printf("  2: Save with just ISBN and fetch details later using 'fetch-metadata'\n");
        printf("Enter choice (1-2): ");
        
        fgets(buffer, sizeof(buffer), stdin);
        int details_choice = atoi(buffer);
        
        if (details_choice == 2) {
            // User wants to save with just ISBN
            printf("Adding book with ISBN only. Using temporary title...\n");
            sprintf(new_book.title, "Book with ISBN: %s", new_book.isbn);
            
            // Add the book to the library with minimal information
            add_book(library, &new_book);
            save_library_to_csv(library, DEFAULT_CSV_FILE);
            printf("\nBook added! Run 'fetch-metadata' to retrieve full details.\n");
            return;
        }
        
        // Otherwise, continue with minimal manual entry
        printf("\nPlease enter minimal required information:\n");
    } else {
        // Manual entry - get ISBN
        printf("Enter ISBN (optional, press enter to skip): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        strncpy(new_book.isbn, buffer, sizeof(new_book.isbn) - 1);
        new_book.isbn[sizeof(new_book.isbn) - 1] = '\0';
    }
    
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
    
    // Get genre (as a string now)
    printf("Enter genre: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    strncpy(new_book.genre, buffer, sizeof(new_book.genre) - 1);
    new_book.genre[sizeof(new_book.genre) - 1] = '\0';
    
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

// Structure to store response data from API calls
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for curl to write response data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

// Function to fetch book information by ISBN from Open Library API
int fetch_book_info_by_isbn(const char* isbn, Book* book) {
    if (!isbn || !*isbn) {
        printf("Error: ISBN is empty\n");
        return 0;
    }
    
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    int success = 0;
    
    // Skip if the ISBN field is empty
    if (strlen(isbn) == 0) {
        return 0;
    }
    
    // Initialize the memory struct
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "https://openlibrary.org/api/books?bibkeys=ISBN:%s&format=json&jscmd=data", isbn);
        
        printf("Fetching data for ISBN: %s\n", isbn);
        printf("URL: %s\n", url);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse JSON response using cJSON
            printf("Response: %s\n", chunk.memory);
            
            cJSON *root = cJSON_Parse(chunk.memory);
            if (root) {
                // The response has the format: {"ISBN:XXXXXXXXXX": { ... book data ... }}
                char isbn_key[30];
                snprintf(isbn_key, sizeof(isbn_key), "ISBN:%s", isbn);
                
                cJSON *book_data = cJSON_GetObjectItem(root, isbn_key);
                if (book_data) {
                    // Extract title
                    cJSON *title = cJSON_GetObjectItem(book_data, "title");
                    if (title && title->valuestring) {
                        strncpy(book->title, title->valuestring, sizeof(book->title) - 1);
                        book->title[sizeof(book->title) - 1] = '\0';
                        success = 1;
                    }
                    
                    // Extract authors (first author only)
                    cJSON *authors = cJSON_GetObjectItem(book_data, "authors");
                    if (authors && authors->type == cJSON_Array) {
                        cJSON *first_author = cJSON_GetArrayItem(authors, 0);
                        if (first_author) {
                            cJSON *name = cJSON_GetObjectItem(first_author, "name");
                            if (name && name->valuestring) {
                                strncpy(book->author, name->valuestring, sizeof(book->author) - 1);
                                book->author[sizeof(book->author) - 1] = '\0';
                            }
                        }
                    }
                    
                    // Extract publication date
                    cJSON *publish_date = cJSON_GetObjectItem(book_data, "publish_date");
                    if (publish_date && publish_date->valuestring) {
                        const char* date_str = publish_date->valuestring;
                        size_t date_len = strlen(date_str);
                        
                        // Try to extract year from the date string (looking for 4 digit year)
                        for (int i = 0; i <= (int)date_len - 4; i++) {
                            if (isdigit(date_str[i]) && isdigit(date_str[i+1]) && 
                                isdigit(date_str[i+2]) && isdigit(date_str[i+3])) {
                                char year_str[5] = {0};
                                strncpy(year_str, date_str + i, 4);
                                int year = atoi(year_str);
                                // Only use years that make sense (1400-2100)
                                if (year >= 1400 && year <= 2100) {
                                    book->year_published = year;
                                    break;
                                }
                            }
                        }
                    }
                    
                    // Extract genre from subjects
                    cJSON *subjects = cJSON_GetObjectItem(book_data, "subjects");
                    if (subjects && subjects->type == cJSON_Array && cJSON_GetArraySize(subjects) > 0) {
                        cJSON *first_subject = cJSON_GetArrayItem(subjects, 0);
                        if (first_subject && first_subject->type == cJSON_Object) {
                            cJSON *subject_name = cJSON_GetObjectItem(first_subject, "name");
                            if (subject_name && subject_name->valuestring) {
                                // Capitalize first letter for consistent formatting
                                char genre_str[50] = {0};
                                strncpy(genre_str, subject_name->valuestring, sizeof(genre_str) - 1);
                                if (genre_str[0] != '\0') {
                                    genre_str[0] = toupper(genre_str[0]);
                                    strncpy(book->genre, genre_str, sizeof(book->genre) - 1);
                                    book->genre[sizeof(book->genre) - 1] = '\0';
                                }
                            }
                        }
                    }
                    
                    // Try to determine number of pages/word count
                    cJSON *num_pages = cJSON_GetObjectItem(book_data, "number_of_pages");
                    if (num_pages && num_pages->type == cJSON_Number) {
                        // Estimate word count based on pages (rough estimate: 250 words per page)
                        book->word_count = num_pages->valueint * 250;
                    }
                    
                    printf("Successfully fetched book data: %s by %s (%d)\n", 
                           book->title, book->author, book->year_published);
                } else {
                    printf("No data found in JSON response for ISBN: %s\n", isbn);
                }
                
                cJSON_Delete(root);
            } else {
                printf("Failed to parse JSON for ISBN: %s\n", isbn);
            }
        }
        
        curl_easy_cleanup(curl);
    }
    
    free(chunk.memory);
    return success;
}

// Function to update library books with metadata from Open Library API
int update_library_with_api_data(Library* library) {
    int updated_count = 0;
    
    // Initialize curl once for all requests
    curl_global_init(CURL_GLOBAL_ALL);
    
    for (int i = 0; i < library->count; i++) {
        Book* book = &library->books[i];
        
        // Skip books without an ISBN
        if (book->isbn[0] == '\0') {
            printf("Skipping book #%d: %s (no ISBN)\n", i+1, book->title);
            continue;
        }
        
        // Skip books that already have metadata retrieved
        if (book->metadata_retrieved) {
            printf("Skipping book #%d: %s (metadata already retrieved)\n", i+1, book->title);
            continue;
        }
        
        printf("Processing book #%d: %s, ISBN: %s\n", i+1, book->title, book->isbn);
        
        // Create a temporary book to store API data
        Book temp_book;
        memset(&temp_book, 0, sizeof(Book));
        
        // Copy the ISBN to the temporary book
        strncpy(temp_book.isbn, book->isbn, sizeof(temp_book.isbn) - 1);
        temp_book.isbn[sizeof(temp_book.isbn) - 1] = '\0';
        
        // Fetch book info from Open Library API
        if (fetch_book_info_by_isbn(book->isbn, &temp_book)) {
            // Update the book data with fetched information
            // Only update if we got actual data
            if (temp_book.title[0] != '\0') {
                strncpy(book->title, temp_book.title, sizeof(book->title) - 1);
                book->title[sizeof(book->title) - 1] = '\0';
            }
            
            if (temp_book.author[0] != '\0') {
                strncpy(book->author, temp_book.author, sizeof(book->author) - 1);
                book->author[sizeof(book->author) - 1] = '\0';
            }
            
            if (temp_book.year_published > 0) {
                book->year_published = temp_book.year_published;
            }
            
            // Update genre if found in API data
            if (temp_book.genre[0] != '\0') {
                strncpy(book->genre, temp_book.genre, sizeof(book->genre) - 1);
                book->genre[sizeof(book->genre) - 1] = '\0';
            }
            
            // Update word count if estimated from page count
            if (temp_book.word_count > 0) {
                book->word_count = temp_book.word_count;
            }
            
            // Mark this book as having its metadata retrieved
            book->metadata_retrieved = 1;
            
            updated_count++;
            printf("Updated book #%d: %s by %s (%d)\n", 
                   i+1, book->title, book->author, book->year_published);
        }
    }
    
    // Clean up curl
    curl_global_cleanup();
    
    return updated_count;
}

void print_usage(const char* program_name) {
    printf("Bookshelf Management System\n");
    printf("Usage: %s [command]\n\n", program_name);
    printf("Commands:\n");
    printf("  add           - Add a new book (interactive)\n");
    printf("  lookup        - Look up a book by title\n");
    printf("  delete        - Delete a book by title\n");
    printf("  list          - List all books in the library\n");
    printf("  fetch-metadata      - Fetch book metadata from Open Library for books with ISBNs\n");
    printf("  fetch-metadata --force - Force update all books with ISBNs, even if already fetched\n");
    printf("  help          - Show this help message\n");
    printf("\nIf no command is given, the program will show all books.\n");
}