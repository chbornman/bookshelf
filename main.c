/*
 * Bookshelf Management System - Self-contained main file
 * This file includes all other source files for direct compilation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the implementation files directly instead of using headers
// This effectively creates a unity build in a single file
#include "book.h"
#include "library.h"

int main(int argc, char *argv[]) {
    printf("Bookshelf Management System\n\n");
    
    Library library;
    initialize_library(&library);
    
    // Load existing library from CSV file
    load_library_from_csv(&library, DEFAULT_CSV_FILE);
    
    // Process command line arguments
    if (argc > 1) {
        const char* command = argv[1];
        
        if (strcmp(command, "add") == 0) {
            interactive_add_book(&library);
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
    
    return 0;
}
