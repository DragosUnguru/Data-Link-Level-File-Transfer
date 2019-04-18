
#ifndef QUEUE
#define QUEUE
#include "utils.h"
/*
    Quick (priority) queue implementation

    The buffers will be represented by
    this data structure so we'll have
    a priority in resending packages.
    
    The queue doesn't accept (index) duplicates
    and prioritizes frames by index.
*/

typedef struct _queue
{
    struct _queue* next;
    pkg package;
}queue;

typedef struct _q_head
{
    queue* front;
    int len;
}q_head;


q_head* new_queue();
int is_empty(q_head* q);
int get_first_missing_pkg(q_head* q);
int* get_window_of_missing(q_head* q, int* size);
void push(q_head* q, pkg info);
pkg pop(q_head* q);
pkg top(q_head* q);
pkg pop_idx(q_head* q, int idx);
pkg get_pkg(q_head* q, int idx);
void destroy_queue(q_head* q);

#endif