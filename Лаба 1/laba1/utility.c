#include "utility.h"
#include "definitions.h"

/*
    Разделяет строку по словам и возвращает их в массиве. Разделение происходит между символами " \t\n\r".
    Также отдельрным словом считается текст, обёрнутый в кавычки, при этом текст вместе с кавычками должен быть
    разделён от остального текста указанными выше разделяющими символами.
*/
void split(char* input_text, char* words[], int* words_count) {
    char* text = (char*)malloc(strlen(input_text) + 1);
    strcpy(text, input_text);

    *words_count = 0;
    char *word = strtok(text, " \t\n\r");
    
    char* temp_words[MAX_WORDS];
    while (word != NULL) {
        temp_words[(*words_count)++] = word;
        word = strtok(NULL, " \t\n\r");
    }

    int id = 0;
    for(int i = 0; i < *words_count; i++) {
        if (temp_words[i][0] == '\"') {
            char local_token[MAX_USER_POMPT_SIZE];
            local_token[0] = '\0';
            for (int j = i; j < *words_count; j++) {
                strcat(strcat(local_token, " "), temp_words[j]);
                if (temp_words[j][strlen(temp_words[j]) - 1] == '\"') {
                    i = j;
                    break;
                }
            }

            if (local_token[strlen(local_token) - 1] == '\"') {
                words[id] = (char*)malloc(strlen(local_token));

                local_token[strlen(local_token) - 1] = '\0';
                strcpy(words[id++], (char*)local_token + 2);
            }
        } else {
            words[id] = (char*)malloc(strlen(temp_words[i]) + 1);
            strcpy(words[id++], temp_words[i]);
        }
    }
    
    free(text);
    *words_count = id;
    words[*words_count] = NULL;
}