#include "log.h"

#include <stdlib.h>
#include <stdio.h>

FILE *log_file = NULL;
bool LogCreated = false;

void log_create (char *fname)
{
    FILE *file;

    if (!LogCreated) {
        file = fopen(fname, "w");
        if (file == NULL){
            LogCreated = false;
            fputs("Error when log file creating. No Log file created.\n", file);
            return;
        }
        else{
            LogCreated = true;
        }
    }

    log_file = file;

}

void log_write (char *message, ...)
{

    va_list args;
    va_start(args, message);
    if (log_file == NULL){
        vfprintf(stderr, message, args);
    }
    else{
        vfprintf(log_file, message, args);
    }
    va_end(args);

}


void log_close(){
    fclose(log_file);
}

// int main() {
//     log_create("./log_test");

//     log_write("Successfully create the log at least\n", 1111);

//     log_close();
//     return 0;
// }