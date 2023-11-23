#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"
#include "sys/param.h"
#include "string.h"

struct node* tasks = NULL;
struct node* first = NULL;

void add(Task* task) {
    void* f = tasks;
    insert(&tasks, task);
    if (!f) first = tasks;
}

Task* pick_next() {
    struct node* pointer = tasks->prev;
    struct node* priority_node = tasks->task->burst ? tasks : tasks->next;
    while (pointer && pointer != tasks) priority_node = (pointer = pointer->prev)->next->task->burst != 0 && pointer->next->task->priority >= priority_node->task->priority ? pointer->next : priority_node;
    return tasks != tasks->next ? tasks->task->burst != 0 ? (tasks = priority_node)->task : (tasks = ((tasks->prev->next = tasks->next)->prev = tasks->prev)->next) ?(tasks = priority_node)->task : NULL : NULL;
}

void schedule() {
    first->next = tasks;
    tasks->prev = first;

    while (pick_next() != NULL) {
        run(tasks->task, MIN(tasks->task->burst, TIME_QUANTUM));
        tasks->task->burst -= MIN(tasks->task->burst, TIME_QUANTUM);
    }
}