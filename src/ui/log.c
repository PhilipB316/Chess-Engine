// log.c

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "log.h"
#include "ui.h"

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
    fprintf(file, "Log created at %s\n", readable_timestamp);
    fprintf(file, "Hello from C!\n");

    fclose(file);
    printf("File created: %s\n", filename);
    return 0;
}

