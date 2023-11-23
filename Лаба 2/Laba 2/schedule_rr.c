#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"
#include "sys/param.h"

struct node* tasks = NULL;
struct node* first = NULL;

void add(Task* task) {
    void* f = tasks;
    insert(&tasks, task);
    if (!f) first = tasks;
}

Task* pick_next() {
    return tasks != tasks->next ? tasks->task->burst != 0 ? (tasks = tasks->next)->task : (tasks = ((tasks->prev->next = tasks->next)->prev = tasks->prev)->next)->task : NULL;
}

void schedule() {
    first->next = tasks;
    tasks->prev = first;
    tasks = first;

    while (pick_next() != NULL) {
        run(tasks->task, MIN(tasks->task->burst, TIME_QUANTUM));
        tasks->task->burst -= MIN(tasks->task->burst, TIME_QUANTUM);
    }
}
