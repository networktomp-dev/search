/**
 * @file range.c
 * @brief Implementation of safe range parsing functions using strtol and strchr.
 */

#include "range.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Safely extracts and converts a numeric range part.
 *
 * @param start The pointer to the start of the number string.
 * @param end The pointer to the end of the number string (or NULL if to end of string).
 * @return The converted integer value, or -1 on failure.
 */
static int safe_strtol_extract(const char *start, const char *end)
{
    // Copy a safe segment into a temporary buffer for strtol.
    // The longest 32-bit int is 10 digits + sign + null terminator. 16 is plenty.
    char temp_buffer[16]; 
    size_t len = (end == NULL) ? strlen(start) : (size_t)(end - start);

    if (len == 0 || len >= sizeof(temp_buffer) || len > 10) {
        return -1; // Invalid length
    }
    
    // Copy the segment and null-terminate it
    strncpy(temp_buffer, start, len);
    temp_buffer[len] = '\0';

    char *endptr;
    // Use strtol for safe conversion and error checking
    long val = strtol(temp_buffer, &endptr, 10);

    // Check if the entire string was consumed and the result is within int bounds
    if (*endptr != '\0' || endptr == temp_buffer || val > INT_MAX || val < 0) {
        return -1; // Conversion error or negative value
    }

    return (int)val;
}

int get_low_range(const char *arg)
{
    const char *delimiter = strchr(arg, '-');
    if (delimiter == NULL) {
        // If there's no hyphen, treat the whole string as the only value.
        // This makes single-line searches possible if the user only provides one number.
        return safe_strtol_extract(arg, NULL);
    }
    
    return safe_strtol_extract(arg, delimiter);
}

int get_high_range(const char *arg)
{
    const char *delimiter = strchr(arg, '-');
    if (delimiter == NULL) {
        // If there's no hyphen, return the same value as the low range.
        return safe_strtol_extract(arg, NULL);
    }
    
    // The high value starts right after the hyphen
    return safe_strtol_extract(delimiter + 1, NULL);
}
