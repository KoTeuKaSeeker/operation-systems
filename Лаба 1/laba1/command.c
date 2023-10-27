#include "command.h"
#include "myshell.h"

Command* commands_ = NULL;
extern char** __environ;

int comporator(const void* a, const void* b) {
    const char* filename1 = *(const char**)a;
    const char* filename2 = *(const char**)b;

    return strcmp(filename1, filename2);
}

// Конструктор структуры Command
Command create_command(const char* command_name, int(*handle_command)(int argc, char** argv)) {
    Command command;
    strcpy(command.command_name, command_name);
    command.handle_command = handle_command;
    return command;
}

// Обработка команды cd
int handle_change_directory_command(int argc, char** argv) {
    if (argc == 0) {
        chdir(".");
    } else {
        if (chdir(argv[0]) != 0) {
            perror("cd");
        } else {
            setenv("PWD", getcwd(NULL, 0), 1); 
        }
    }

    return true;
}

// Обработка команды clear
int handle_clear_command(int argc, char** argv) {
        printf("\x1B[2J"); // очистка экрана
        printf("\x1B[H");  // перенос курсора в начало

        return true;
}

// Обработка команды dir
int handle_directory_command(int argc, char** argv) {
        struct dirent* entry;
        DIR* dir = (argc == 0 ? opendir(".") : opendir(argv[0]));

        if (dir == NULL) {
            perror("opendir");
            return false;
        }

        char* filenames[MAX_DIRECTORIES];
        int files_cnt = 0;

        while ((entry = readdir(dir)) != NULL) {
            filenames[files_cnt++] = strdup(entry->d_name);
        }

        closedir(dir);

        qsort(filenames, files_cnt, sizeof(char*), comporator);
        for (int i = 0; i < files_cnt; i++) {
            printf("%s\n", filenames[i]);
            free(filenames[i]);
        }

        return true;
}

// Обработка команды environ
int handle_environ_command(int argc, char** argv) {
    char** env = __environ;
    while (*env) {
        printf("%s\n", *env);
        env++;
    }

    return true;
}

// Обработка команды echo
int handle_echo_command(int argc, char** argv) {
    if (file_text == NULL) {
        for(int i = 0; i < argc; i++)
            printf("%s ", argv[i]);
        printf("\n");
    } else {
        printf((isatty(fileno(stdout)) ? "%s\n" : "%s"), file_text); // isatty(fileno(stdout)) истинно, если поток ввывод является терминалом 
    }

    return true;
}

// Обработка команды help
int handle_help_command(int argc, char** argv) {
    pid_t external_pid = fork();

    if(external_pid == 0) {

        int pipefd[2];

        if (pipe(pipefd) == -1) {
            fprintf(stderr, "Pipe create error\n");
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();

        if (child_pid == -1) {
            fprintf(stderr, "Help fork error\n");
            exit(EXIT_FAILURE);
        }

        if (child_pid == 0) {
            close(STDOUT_FILENO);

            int new_fd = dup(pipefd[1]);
            if (new_fd == -1) {
                fprintf(stderr, "dup in help error\n");
                kill(getppid(), SIGTERM);
                exit(EXIT_FAILURE);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            int fd = open("readme", O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "README open error\n");
                kill(getppid(), SIGTERM);
                exit(EXIT_FAILURE);
            }

            struct stat file_stat;
            if (fstat(fd, &file_stat) == -1) {
                fprintf(stderr, "Fstat in help error\n");
                close(fd);
                kill(getppid(), SIGTERM);
                exit(EXIT_FAILURE);
            }

            char buffer[file_stat.st_size];
            ssize_t n;
            while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
                ssize_t written = write(STDOUT_FILENO, buffer, n);
                if (written == -1) {
                    fprintf(stderr, "Write in help error\n");
                    kill(getppid(), SIGTERM);
                    exit(EXIT_FAILURE);
                }
            }

            close(fd);

            exit(EXIT_SUCCESS);
        } else {
            close(pipefd[1]);

            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execlp("more", "more", (char *)NULL);
            exit(EXIT_FAILURE);
        }
    } else {
        waitpid(external_pid, NULL, 0);
        return true;
    }
    return true;
}

// Обработка команды pause
int handle_pause_command(int argc, char** argv) {
    printf("Press Enter to continue...");
    getchar();

    return true;
}

// Обработка команды quit
int handle_quit_command(int argc, char** argv) {
    exit(0);
    return 0;
}

int handle_sleep_command(int argc, char** argv) {
    sleep(atof(argv[0]));
    return 0;
}

/*
    Возвращает массив существующих комманд. 
    
    P.S. Значение COUNT_COMMANDS должно быть равно количеству комманд в массиве commands_.
    То есть, если вы хотите добавить новую комманду, нужно увеличить значение COUNT_COMMANDS на 1.
*/
Command* get_commands() {
    if (!commands_) {
        commands_ = malloc(sizeof(Command) * COUNT_COMMANDS);
        
        int id = 0;
        commands_[id++] = create_command("cd", handle_change_directory_command);
        commands_[id++] = create_command("clr", handle_clear_command);
        commands_[id++] = create_command("dir", handle_directory_command);
        commands_[id++] = create_command("environ", handle_environ_command);
        commands_[id++] = create_command("echo", handle_echo_command);
        commands_[id++] = create_command("help", handle_help_command);
        commands_[id++] = create_command("pause", handle_pause_command);
        commands_[id++] = create_command("quit", handle_quit_command);
        commands_[id++] = create_command("sleep", handle_sleep_command);
    }

    return commands_;
}