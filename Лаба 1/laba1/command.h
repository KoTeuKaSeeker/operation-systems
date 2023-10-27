#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stddef.h>
#include <signal.h>

#include "definitions.h"
#include "utility.h"

#define COUNT_COMMANDS 9

typedef struct {
    char command_name[25];
    int(*handle_command)(int argc, char** argv);
} Command;

extern Command* commands_;

int comporator(const void* a, const void* b);
Command create_command(const char* command_name, int(*handle_command)(int argc, char** argv));

//Комманды
int handle_change_directory_command(int argc, char** argv);
int handle_clear_command(int argc, char** argv);
int handle_directory_command(int argc, char** argv);
int handle_environ_command(int argc, char** argv);
int handle_echo_command(int argc, char** argv);
int handle_help_command(int argc, char** argv);
int handle_pause_command(int argc, char** argv);
int handle_quit_command(int argc, char** argv);

Command* get_commands();

#endif