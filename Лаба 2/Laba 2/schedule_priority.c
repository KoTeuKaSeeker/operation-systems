#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"

struct node* tasks = NULL;

void add(Task* task) {
    insert(&tasks, task);
}

Task* selected = NULL;

Task* pick_next() {
    if (selected != NULL) delete(&tasks, selected);
    if (tasks == NULL) return NULL;

    struct node* pointer = tasks;
    Task* max_priority = tasks->task;

    while (pointer != NULL) {
        if (pointer->task->priority > max_priority->priority) max_priority = pointer->task;
        pointer = pointer->next;
    }

    return max_priority;
}

void schedule() {
    while ((selected = pick_next()) != NULL) run(selected, selected->burst);
}
