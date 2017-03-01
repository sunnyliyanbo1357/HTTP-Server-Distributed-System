#ifndef HAVELOG
#define HAVELOG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>


FILE *log_file;

extern bool LogCreated;      // keeps track whether the log file is created or not

void log_create (char *fname);    // logs a message to LOGFILE
void log_write (char *message, ...); // logs a message; execution is interrupted
void log_close(); // close log file

#endif