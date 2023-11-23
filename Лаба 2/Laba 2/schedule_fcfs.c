#include "schedulers.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include <time.h>

struct node* tasks = NULL;

// add a task to the list 
void add(Task* task) {
    insert(&tasks, task);
}

Task* selected = NULL;

Task* pick_next() {
    if (selected) delete(&tasks, selected);
    return !tasks ? NULL : tasks->task;
}

void schedule() {
    while ((selected = pick_next()) != NULL) run(selected, selected->burst);
}