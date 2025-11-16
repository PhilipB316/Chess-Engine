// log.c

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>

#include "log.h"
#include "../interface/ui.h"

static char filename[50];
static FILE* file;

bool touch_log_file(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d-%H-%M-%S", local);
    mkdir("../log", 0755);
    snprintf(filename, sizeof(filename), "../log/log-%s.txt", timestamp);

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error creating file");
        return 1;
    }

    fputs(header, file);

    char readable_timestamp[30];
    strftime(readable_timestamp, sizeof(readable_timestamp), "%Y-%m-%d %H:%M:%S", local);
    fprintf(file, "Log file created at %s\n\n", readable_timestamp);
    fclose(file);
    return 0;
}

void log_message(const char* message) {
    file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }
    fputs(message, file);
    fclose(file);
}

void write_log_pgn_header(bool playing_as_white) {
    file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file for appending");
        return;
    }
    fprintf(file, "[Event \"TessMax Game\"]\n");
    fprintf(file, "[Site \"Local\"]\n");
    fprintf(file, "[Date \"%s\"]\n", DATE_STRING);
    fprintf(file, "[Round \"NA\"]\n");
    fprintf(file, "[White \"%s\"]\n", playing_as_white ? "User" : "TessMax");
    fprintf(file, "[Black \"%s\"]\n", playing_as_white ? "TessMax" : "User");
    fprintf(file, "[Result \"*\"]\n\n");
    fclose(file);
}

void write_result_to_log(int white_result)
{
    // delay 500 milliseconds to let gui thread finish writing moves
    struct timespec ts = {0, 50 * 1000000}; // 50 milliseconds
    nanosleep(&ts, NULL);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return;
    }

    // Read all lines into memory
    char lines[1024][256];
    int line_count = 0;
    while (fgets(lines[line_count], sizeof(lines[line_count]), file) && line_count < 1024) {
        line_count++;
    }
    fclose(file);

    // Prepare result string
    const char *result_str;
    if (white_result == 1) {
        result_str = "1-0";
    } else if (white_result == 0) {
        result_str = "0-1";
    } else {
        result_str = "1/2-1/2";
    }

    // Update [Result "..."] line if found
    for (int i = 0; i < line_count; i++) {
        if (strncmp(lines[i], "[Result \"", 9) == 0) {
            snprintf(lines[i], sizeof(lines[i]), "[Result \"%s\"]\n", result_str);
        }
    }

    // Write all lines back to file
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }
    for (int i = 0; i < line_count; i++) {
        fputs(lines[i], file);
    }
    // Optionally, write result at end of file
    fprintf(file, "%s\n", result_str);
    fclose(file);
}
