/**
* @file log.h
* @brief Logging utilities for chess engine
* @author Philip Brand
* @date 2025-10-22
*/

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

/**
 * @brief Opens new log file for writing
 */
bool touch_log_file(void);

/**
 * @brief Appends a message to the log file
 * @param message The message to log
 */
void log_message(const char* message);

#endif // LOG_H
