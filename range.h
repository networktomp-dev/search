/**
 * @file range.h
 * @brief Header for line range parsing utility functions.
 */
#ifndef RANGE_H
#define RANGE_H

#include <stdlib.h>
#include <limits.h>

/**
 * @brief Parses the left (lower) value from a range string (e.g., "50-75").
 *
 * @param args The range string.
 * @return The integer value of the left side, or -1 on error.
 */
int get_low_range(const char *arg);

/**
 * @brief Parses the right (upper) value from a range string (e.g., "50-75").
 *
 * @param args The range string.
 * @return The integer value of the right side, or -1 on error.
 */
int get_high_range(const char *arg);

#endif // RANGE_H
