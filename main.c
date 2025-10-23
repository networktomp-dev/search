/**
 * @file main.c
 * @brief A command-line file search utility.
 *
 * Refactored to use standard argument parsing (getopt_long) and decoupled
 * search/print logic for improved efficiency and maintainability.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#include "range.h"
#include "nerror.h"

// --- Constants and Definitions ---

#define MAX_LINE_LENGTH 2048
#define MAX_TERM_LENGTH 128

// Option bitmasks
#define OPTION_IGNORE 	(1 << 0) // 0b00000001
#define OPTION_ISOLATE 	(1 << 1) // 0b00000010
#define OPTION_LINES	(1 << 2) // 0b00000100
#define OPTION_RANGE	(1 << 3) // 0b00001000
#define OPTION_REMOVE	(1 << 4) // 0b00010000
#define OPTION_SAVE	    (1 << 5) // 0b00100000

// --- Utility Functions ---

/**
 * @brief Checks if a character is a non-word boundary character (part of a word).
 * @param c The character to check.
 * @return 1 if c is a letter, digit, or underscore, 0 otherwise.
 */
int is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

/**
 * @brief Searches for a term within a line, respecting case-sensitivity and isolation.
 *
 * @param line The line buffer to search.
 * @param term The search term.
 * @param options The option field flags.
 * @return A pointer to the start of the match in the line, or NULL if no match is found.
 */
char *search_line(const char *line, const char *term, uint8_t options)
{
    size_t term_len = strlen(term);
    const char *current_line_ptr = line;
    const char *match_ptr = NULL;

    // The inner search loop
    while (*current_line_ptr != '\0') {
        int match = 1;
        
        // 1. Check if the first character matches (with optional case-insensitivity)
        if (!((options & OPTION_IGNORE) ? (toupper(*current_line_ptr) == toupper(*term)) : (*current_line_ptr == *term))) {
            current_line_ptr++;
            continue;
        }

        // 2. Check if the remaining characters match
        for (size_t i = 1; i < term_len; i++) {
            if (!((options & OPTION_IGNORE) ? (toupper(current_line_ptr[i]) == toupper(term[i])) : (current_line_ptr[i] == term[i]))) {
                match = 0;
                break;
            }
        }

        if (match) {
            // 3. Match found. Now check for isolation if required.
            if (options & OPTION_ISOLATE) {
                
                // Check character immediately before the match (if it exists)
                int start_ok = (current_line_ptr == line) || !is_word_char(*(current_line_ptr - 1));
                
                // Check character immediately after the match (if it exists)
                int end_ok = (current_line_ptr[term_len] == '\0' || !is_word_char(current_line_ptr[term_len]));
                
                if (start_ok && end_ok) {
                    match_ptr = current_line_ptr;
                    // We found an isolated match, return the pointer
                    return (char *)match_ptr;
                }
            } else {
                // Not isolated search, any match is fine
                match_ptr = current_line_ptr;
                return (char *)match_ptr;
            }
        }
        
        // Move to the next character to start the next comparison
        current_line_ptr++;
    }

    return NULL; // No match found in the entire line
}

// --- Main Program ---

void print_help(void) {
    puts("Search help:\n\tUSAGE: search [OPTION]... TERM FILE");
    puts("\n\t-h, --help\t\tShow this help dialog");
    puts("\t-i, --ignore-case\tSearch is not case sensitive");
    puts("\t-I, --isolate\t\tOnly return a word where it is an exact match (not part of a compound word).");
    puts("\t-l, --lines\t\tDisplay line numbers and the starting position of the word.");
    puts("\t-r, --range NUM-NUM\tDisplay results only from a given range of lines (e.g., -r 50-75).");
    puts("\t-R, --remove-dupes\tOnly shows the line once, regardless of matches (Not fully implemented yet).");
    puts("\t-s, --save FILE\t\tSave results to a file.");
    puts("\n\tEG: search Port /etc/ssh/sshd_config | grep 22");
}

int main(int argc, char *argv[])
{
    // --- Argument Parsing Setup ---
    
    // Default values
    uint8_t option_field = 0;
    char *save_filepath = NULL;
    char *range_arg = NULL;
    char *search_term = NULL;
    char *search_file = NULL;

    int lowerrange = 0;
    int upperrange = 0;

    // getopt_long configuration
    int c;
    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"ignore-case", no_argument, 0, 'i'},
        {"isolate", no_argument, 0, 'I'},
        {"lines", no_argument, 0, 'l'},
        {"range", required_argument, 0, 'r'},
        {"remove-dupes", no_argument, 0, 'R'},
        {"save", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    
    // Parse arguments using getopt_long
    while ((c = getopt_long(argc, argv, "hIiIr:lRs:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'i':
                FAIL_IF_R_M(option_field & OPTION_IGNORE, 1, stderr, "ERROR: You can only employ a flag once (--ignore-case)\n");
                option_field |= OPTION_IGNORE;
                break;
            case 'I':
                FAIL_IF_R_M(option_field & OPTION_ISOLATE, 1, stderr, "ERROR: You can only employ a flag once (--isolate)\n");
                option_field |= OPTION_ISOLATE;
                break;
            case 'l':
                FAIL_IF_R_M(option_field & OPTION_LINES, 1, stderr, "ERROR: You can only employ a flag once (--lines)\n");
                option_field |= OPTION_LINES;
                break;
            case 'r':
                FAIL_IF_R_M(option_field & OPTION_RANGE, 1, stderr, "ERROR: You can only employ a flag once (--range)\n");
                range_arg = optarg;
                option_field |= OPTION_RANGE;
                break;
            case 'R':
                FAIL_IF_R_M(option_field & OPTION_REMOVE, 1, stderr, "ERROR: You can only employ a flag once (--remove-dupes)\n");
                option_field |= OPTION_REMOVE;
                break;
            case 's':
                FAIL_IF_R_M(option_field & OPTION_SAVE, 1, stderr, "ERROR: You can only employ a flag once (--save)\n");
                save_filepath = optarg;
                option_field |= OPTION_SAVE;
                break;
            case '?': // getopt_long handles unknown option errors and prints a message
                return 1;
            default:
                return 1;
        }
    }

    // --- Positional Argument Checks (TERM and FILE) ---
    
    // We expect exactly two non-option arguments left: TERM and FILE
    if (argc - optind < 2) {
        if (argc - optind == 1) {
            fprintf(stderr, "ERROR: Missing search file path.\n");
        } else {
            fprintf(stderr, "USAGE: search [OPTION]... TERM FILE\n");
            fprintf(stderr, "Try 'search --help' for more information\n");
        }
        return 1;
    }
    
    search_term = argv[optind];
    search_file = argv[optind + 1];

    // --- Range Processing ---

    if (option_field & OPTION_RANGE) {
        lowerrange = get_low_range(range_arg);
        upperrange = get_high_range(range_arg);
        
        // Handle range parsing errors
        if (lowerrange < 0 || upperrange < 0) {
            fprintf(stderr, "ERROR: Invalid range format. Please use NUM-NUM or a non-negative number.\n");
            return 1;
        }

        // Sort the range values
        if (upperrange < lowerrange) {
            int intholder = upperrange;
            upperrange = lowerrange;
            lowerrange = intholder;
        }
    }

    // --- File Handling Setup ---
    
    FILE *searchfile = fopen(search_file, "r");
    FAIL_IF_R_M(searchfile == NULL, 1, stderr, "search: Could not open search file.\n");

    FILE *file_stream = stdout; // Default output stream
    if (option_field & OPTION_SAVE) {
        file_stream = fopen(save_filepath, "w");
        FAIL_IF_R_M(file_stream == NULL, 1, stderr, "search: Could not open save file.\n");
    }

    // --- Status Output ---

    fprintf(stderr, "Searching for \"%s\" in %s\n", search_term, search_file);
    if (option_field & OPTION_ISOLATE) fprintf(stderr, "Isolating matches...\n");
    if (option_field & OPTION_IGNORE) fprintf(stderr, "Ignoring cases...\n");
    if (option_field & OPTION_LINES) fprintf(stderr, "Including line numbers/positions...\n");
    if (option_field & OPTION_REMOVE) fprintf(stderr, "Removing duplicate lines...\n");
    if (option_field & OPTION_RANGE) fprintf(stderr, "Showing results in a range: %d-%d...\n", lowerrange, upperrange);
    if (option_field & OPTION_SAVE) fprintf(stderr, "Saving results to %s...\n", save_filepath);
    fputc('\n', stderr);

    // --- Core Search Loop ---

    char linebuff[MAX_LINE_LENGTH];
    int linecount = 1;
    unsigned int resultstracker = 0;

    // Check if search term is too long
    FAIL_IF_R_M(strlen(search_term) >= MAX_TERM_LENGTH, 1, stderr, "ERROR: Search term is too long.\n");

    while (fgets(linebuff, MAX_LINE_LENGTH, searchfile)) {
        
        // 1. Range check
        if ((option_field & OPTION_RANGE) && (linecount < lowerrange || linecount > upperrange)) {
            linecount++;
            continue;
        }

        // 2. Search for all matches in the current line
        int matches_on_line = 0;
        char *search_start = linebuff;
        
        // Loop while matches are found, starting the next search after the last match
        while ((search_start = search_line(search_start, search_term, option_field)) != NULL) {
            
            // Match found!
            matches_on_line++;
            
            // 3. Print the prefix (Line number/Position) if required
            if (option_field & OPTION_LINES) {
                // Calculate position based on the start of the line
                int position = (int)(search_start - linebuff) + 1;
                fprintf(file_stream, "LINE %d, POS %d: ", linecount, position);
            }

            // 4. Print the line content
            fprintf(file_stream, "%s", linebuff);
            resultstracker++;
            
            // 5. Handle OPTION_REMOVE: if we show the line once, break the inner search loop
            if (option_field & OPTION_REMOVE) {
                break;
            }

            // Move search_start past the found term to look for the next match on the same line
            search_start += strlen(search_term);
        }

        linecount++;
    }

    // --- Cleanup and Summary ---

    fclose(searchfile);
    if (option_field & OPTION_SAVE) {
        fprintf(stderr, "\n%u results written to %s.\n", resultstracker, save_filepath);
        fclose(file_stream);
    } else {
        fprintf(stderr, "\n%u results written to stdout.\n", resultstracker);
    }

    return 0;
}
