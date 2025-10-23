/**
 * @file nerror.h
 * @brief Simplified and safer error handling macros.
 *
 * NOTE: The original version had incorrect header guards for standard libraries.
 * We are simplifying this to a single, safer macro.
 */
#ifndef NERROR_H
#define NERROR_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Checks an expression and, if true, prints a message to a stream and
 * returns a specified value.
 *
 * This uses a safer `do { ... } while(0)` wrapper to ensure the macro behaves
 * correctly in `if/else` statements without requiring braces.
 *
 * @param EXP The expression to check.
 * @param RETURN_VALUE The value to return (can be a variable, NULL, or 0).
 * @param STREAM The file stream (e.g., stderr).
 * @param MSG The error message string literal.
 */
#ifndef FAIL_IF_R_M
#define FAIL_IF_R_M(EXP, RETURN_VALUE, STREAM, MSG) \
    do {                                            \
        if (EXP) {                                  \
            fprintf(STREAM, "%s", MSG);             \
            return RETURN_VALUE;                    \
        }                                           \
    } while (0)
#endif // FAIL_IF_R_M

// Keeping the original name for compatibility, but its safer sibling above is preferred.
#define FAIL_IF_E_M(EXP, EXIT_STATUS, STREAM, MSG) \
    do {                                            \
        if (EXP) {                                  \
            fprintf(STREAM, "%s", MSG);             \
            exit(EXIT_STATUS);                      \
        }                                           \
    } while (0)

#endif // NERROR_H
