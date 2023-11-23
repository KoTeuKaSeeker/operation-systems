/**
 * Driver.c
 *
 * Schedule is in the format
 *
 *  [name] [priority] [CPU burst]
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"

#define SIZE 100

int main(int argc, char *argv[]) {
    FILE *in;
    char *temp;
    char task[SIZE];

    char *name;
    int priority;
    int burst;

    in = fopen(argv[1],"r");
    //in = fopen("schedule.txt", "r");

    Task* tasks_array[MAX_TASKS];
    int count_tasks = 0;
    struct node* temp_list;
    while (fgets(task,SIZE,in) != NULL) {
        temp = strdup(task);
        name = strsep(&temp,",");
        priority = atoi(strsep(&temp,","));
        burst = atoi(strsep(&temp,","));

        // add the task to the scheduler's list of tasks
        add_task(&temp_list, name, priority, burst);
        tasks_array[count_tasks] = temp_list->task;
        count_tasks++;

        free(temp);
    }

    while (temp_list != NULL) {
        add(temp_list->task);
        delete(&temp_list, temp_list->task);
    }

    fclose(in);

    // invoke the scheduler
    schedule();

    float avarage_turnarouond_time = 0;
    float avarage_wait_time = 0;
    float avarage_response_time = 0;
    for(int x = 0; x < count_tasks; x++) {
        avarage_turnarouond_time += tasks_array[x]->time_end - tasks_array[x]->time_start;
        avarage_wait_time += tasks_array[x]->wait_time;
        avarage_response_time += tasks_array[x]->time_start;
    }
    avarage_turnarouond_time /= count_tasks;
    avarage_wait_time /= count_tasks;
    avarage_response_time /= count_tasks;
        
    printf("\nAverage turnaround time: %f", avarage_turnarouond_time);
    printf("\nAverage wait time: %f", avarage_wait_time);
    printf("\nAverage response time: %f\n", avarage_response_time);

    return 0;
}
