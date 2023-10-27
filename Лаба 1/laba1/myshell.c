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
#include <string.h>

#include "definitions.h"
#include "myshell.h"
#include "utility.h"
#include "command.h"

extern char** __environ;
char* file_text = NULL;
int count_words_without_file = 0;

int get_streams(char* words[], int* words_count, int* console_io_streams, int* io_streams) {
    memcpy(io_streams, console_io_streams, sizeof(int) * 2);

    int new_words_count = *words_count;

    // Обработка вывода результата
    for(int i = 0; i < *words_count; i++) {
        if (strcmp(words[i], "<") == 0 || strcmp(words[i], "<<") == 0) {
            if (i + 1 < *words_count) {
                int instream = open(words[i + 1], O_RDONLY, 0644);
                if (instream < 0) {
                    perror("freopen");
                    return false;
                } else {
                    io_streams[0] = instream;
                }
                if(new_words_count == *words_count)
                    new_words_count = i;
            } else {
                perror("freopen: not enought arguments.");
                return false;
            }
        }

        if (strcmp(words[i], ">") == 0 || strcmp(words[i], ">>") == 0) {
            if (i + 1 < *words_count) {
                int outstream = open(words[i + 1], (strcmp(words[i], ">") == 0 ? O_CREAT | O_WRONLY | O_TRUNC : O_CREAT | O_RDWR | O_APPEND), 0644);
                if (outstream < 0) {
                    perror("freopen");
                    return false;
                } else {
                    io_streams[1] = outstream;
                }
                if(new_words_count == *words_count)
                    new_words_count = i;
            } else {
                perror("freopen: not enought arguments.");
                return false;
            }
        }
    }

    *words_count = new_words_count;
    words[new_words_count] = NULL;
    return true;
}

/*
    Обрабатывается комманду, введённую пользователем.
*/
int handle_command(char* words[], int words_count) {
    Command* commands = get_commands();
    for (int i = 0; i < COUNT_COMMANDS; i++) {
        if (strcmp(words[0], commands[i].command_name) == 0) {
            return commands[i].handle_command(words_count - 1, words + 1);
        }
    }

    return false;
}

pid_t background_mode(char* words[], int* words_count) {
    if(strcmp(words[*words_count - 1], "&") == 0) {
        pid_t child_pid = fork();
        
        if(child_pid == 0) {
            *words_count -= 1;
            words[*words_count] = NULL;
        } else {
            *words_count = 0;
        }

        return child_pid;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    char input[MAX_USER_POMPT_SIZE];
    char* words[MAX_WORDS];
    int words_count = 0;

    // Если список комманд вводится через файл
    if (argc > 1) {
        if (freopen(argv[1], "r", stdin) == NULL) {
            perror("freopen");
            return 1;
        }
    }

    pid_t background_mode_pid = 1;
    int console_streams[2] = {dup(fileno(stdin)), dup(fileno(stdout))};
    int id = 0;
    // Основной цикл
    while (1) {
        // Если текущий процесс выполнялся в фоном режиме, то завершаем цикл
        if (background_mode_pid == 0) {
            break;
        }

        dup2(console_streams[0], fileno(stdin)); // Возвращение потока ввода в консоль
        dup2(console_streams[1], fileno(stdout)); // Возвращение потока вывода в консоль
        
        // Вывод текущей дериктории в консоль
        if (argc <= 1) {
            char* dir = getcwd(NULL, 0);
            printf("%s: ", dir);
            free(dir);
        }

        // Считывание команды пользователя
        if (fgets(input, MAX_USER_POMPT_SIZE, stdin) == NULL) {
            break;
        }

        // Разбиение комманды на словаS
        split(input, words, &words_count);

        // Обработка фонового режима &
        background_mode_pid = background_mode(words, &words_count);

        if (background_mode_pid == 0) {
            console_streams[0] = open("/dev/null", O_RDONLY, 0644);
            console_streams[1] = open("/dev/null", O_WRONLY, 0644);

            dup2(console_streams[0], fileno(stdin)); // Возвращение потока ввода в консоль
            dup2(console_streams[1], fileno(stdout)); // Возвращение потока вывода в консоль
        }

        // Если что-то было введено
        if (words_count > 0) {
            int io_streams[2];
            if (!get_streams(words, &words_count, console_streams, io_streams)) {
                perror("iostreams");
                continue;
            }

            dup2(io_streams[0], STDIN_FILENO); // Перенаправление потока ввода
            dup2(io_streams[1], STDOUT_FILENO); // Перенаправление потока вывода

            file_text = NULL;

            //Если поток ввода перенаправлен, то текст из файла нужно записать в массив words
            if (io_streams[0] != console_streams[0]) {
                char c;
                int pointer = 0;
                while (pointer < MAX_USER_POMPT_SIZE && (c = getchar()) != EOF) {
                    input[pointer++] = c;
                }

                file_text = input;

                /* 
                    Сброс состояния потока ввода. Если так не сделать, поток ввода будет постоянно 
                    при попытки считывания говорить о достижении конца файла и возвращать EOF или NULL.
                */
                clearerr(stdin);

                int file_words_count;
                split(input, words + words_count, &file_words_count);
                
                words_count += file_words_count;
            }

            // Обработка комманды
            if (handle_command(words, words_count)) {
                continue;
            }

            
            
            /*
                Если код дошёл до этого места, значит не было найдено соответсвтие ни одной комманде из
                существующих. Поэтому далее идёт попытка вызвать программу, введённую пользователем с передачей 
                ей аргументов.
            */

            pid_t child_pid = fork();

            if (child_pid == 0) {
                // Вызывается программа, введённая пользователем
                execvp(words[0], words);
                perror("execvp");
                exit(1);
            } else if (child_pid > 0) {
                if (words[words_count-1][0] != '&') {
                    waitpid(child_pid, NULL, 0); // Ожидание завершения дочернего процесса
                }
            } else {
                perror("fork");
            }
        }
        id++;
    }

    return 0;
}