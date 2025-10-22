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

/**
 * @brief Writes the PGN header to the log file
 * @param playing_as_white Boolean indicating if the user is playing as white
 */
void write_log_pgn_header(bool playing_as_white);

/**
 * @brief Logs the final game result in PGN format
 * @param white_result 1 for white win, 0 for loss, anything else for draw
 */
void write_result_to_log(int white_result);

#endif // LOG_H
