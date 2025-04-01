/*
 * Bookshelf Management System - Self-contained main file
 * This file includes all other source files for direct compilation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>  // Include curl for global init/cleanup

// Include the implementation files directly instead of using headers
// This effectively creates a unity build in a single file
#include "book.h"
#include "library.h"

int main(int argc, char *argv[]) {
    printf("Bookshelf Management System\n\n");
    
    // Initialize curl at program start
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    Library library;
    initialize_library(&library);
    
    // Load existing library from CSV file
    load_library_from_csv(&library, DEFAULT_CSV_FILE);
    
    // Process command line arguments
    if (argc > 1) {
        const char* command = argv[1];
        
        if (strcmp(command, "add") == 0) {
            interactive_add_book(&library);
            
            // Offer to immediately fetch metadata if book has ISBN
            int has_isbn = 0;
            for (int i = 0; i < library.count; i++) {
                if (library.books[i].isbn[0] != '\0' && !library.books[i].metadata_retrieved) {
                    has_isbn = 1;
                    break;
                }
            }
            
            if (has_isbn) {
                char choice[10];
                printf("\nWould you like to fetch metadata for books with ISBNs now? (y/n): ");
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0; // Remove newline
                
                if (choice[0] == 'y' || choice[0] == 'Y') {
                    printf("Fetching metadata...\n");
                    int updated = update_library_with_api_data(&library);
                    if (updated > 0) {
                        printf("Successfully updated %d books with metadata.\n", updated);
                        save_library_to_csv(&library, DEFAULT_CSV_FILE);
                    } else {
                        printf("No books were updated.\n");
                    }
                }
            }
        }
        else if (strcmp(command, "lookup") == 0) {
            interactive_lookup_book(&library);
        }
        else if (strcmp(command, "delete") == 0) {
            interactive_delete_book(&library);
        }
        else if (strcmp(command, "list") == 0) {
            print_library(&library);
        }
        else if (strcmp(command, "fetch-metadata") == 0) {
            printf("Attempting to update library with metadata from Open Library API...\n");
            
            // Check for force flag to update all books regardless of metadata_retrieved status
            int force_update = 0;
            if (argc > 2 && strcmp(argv[2], "--force") == 0) {
                force_update = 1;
                printf("Force flag detected. Will attempt to update all books with ISBNs.\n");
                
                // Reset metadata_retrieved flags if forcing update
                for (int i = 0; i < library.count; i++) {
                    if (library.books[i].isbn[0] != '\0') {
                        library.books[i].metadata_retrieved = 0;
                    }
                }
            }
            
            int updated = update_library_with_api_data(&library);
            if (updated > 0) {
                printf("Successfully updated %d books with metadata.\n", updated);
                save_library_to_csv(&library, DEFAULT_CSV_FILE);
            } else {
                if (force_update) {
                    printf("No books were updated. Ensure your books have valid ISBNs.\n");
                } else {
                    printf("No books were updated. Use --force to update all books with ISBNs.\n");
                }
            }
        }
        else if (strcmp(command, "help") == 0) {
            print_usage(argv[0]);
        }
        else {
            printf("Unknown command: %s\n", command);
            print_usage(argv[0]);
        }
    }
    else {
        // If no arguments, just show the library contents
        print_library(&library);
    }
    
    free_library(&library);
    
    // Clean up curl at program end
    curl_global_cleanup();
    
    return 0;
}
