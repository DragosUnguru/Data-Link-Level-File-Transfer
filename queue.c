#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "queue.h"

q_head* new_queue() {
    q_head* q = (q_head*) malloc(sizeof(q_head));

    q->len = 0;
    q->front = NULL;

    return q;
}

int is_empty(q_head* q) {
    return (q->len == 0) ? 1 : 0;
}

void push(q_head* q, pkg info) {
    queue* new_node = (queue*) malloc(sizeof(queue));

    memcpy(&(new_node->package), &info, sizeof(pkg));
    new_node->next = NULL;

    if (q->front == NULL) {
        q->front = new_node;
        (q->len)++;
        return;
    }

    queue* tmp = q->front;

    if (tmp->package.idx == info.idx) {
        return;
    }

    if (tmp->package.idx >= info.idx) {
        q->front = new_node;
        new_node->next = tmp;

        (q->len)++;
        return;
    }

    while((tmp->next != NULL) && (tmp->next->package.idx <= info.idx)) {
        if (tmp->next->package.idx == info.idx) {
            return;
        }

        tmp = tmp->next;
    }
    
    new_node->next = tmp->next;
    tmp->next = new_node;

    (q->len)++;
}

pkg pop(q_head* q) {
    if (is_empty(q)) {
        pkg sentinel;
        sentinel.idx = -1;

        return sentinel;
    }
    pkg ret;
    memcpy(&ret, &(q->front->package), sizeof(pkg));

    queue* tmp = q->front;
    q->front = tmp->next;

    (q->len)--;
    free(tmp);

    return ret;
}

pkg pop_idx(q_head* q, int idx) {
    pkg result;
    result.idx = -1;

    if (is_empty(q)) {
        return result;
    }

    if (q->front->package.idx == idx) {
        queue* tmp = q->front;
        memcpy(&result, &(tmp->package), sizeof(pkg));

        q->front = tmp->next;
        (q->len)--;

        free(tmp);
        return result;
    }

    queue* tmp = q->front;
    queue* stalker = tmp;

    while(tmp->package.idx != idx) {
        if (tmp->next == NULL) {
            return result;
        }
        if ((tmp->next != NULL) && (tmp->next->package.idx > idx)) {
            return result;
        }
        stalker = tmp;
        tmp = tmp->next;
    }
    stalker->next = tmp->next;
    (q->len)--;

    memcpy(&result, &(tmp->package), sizeof(pkg));
    free(tmp);

    return result;
}

pkg get_pkg(q_head* q, int idx) {
    pkg result;
    result.idx = -1;

    if (q->front == NULL) {
        return result;
    }

    queue* tmp = q->front;

    while(tmp->package.idx != idx) {
        if (tmp->next == NULL) {
            return result;
        }
        if ((tmp->next != NULL) && (tmp->next->package.idx > idx)) {
            return result;
        }
        tmp = tmp->next;
    }

    memcpy(&result, &(tmp->package), sizeof(pkg));

    return result;
}

int get_first_missing_pkg(q_head* q) {
    int res = 0;

    if (is_empty(q)) {
        return res;
    }

    if (q->front->package.idx > 0) {
        return res;
    }

    queue* tmp = q->front;

    while ((tmp->next != NULL) && (tmp->package.idx + 1 == tmp->next->package.idx)) {
        tmp = tmp->next;
    }

    res = tmp->package.idx + 1;

    return res;
}

int* get_window_of_missing(q_head* q, int* size) {
    int* res = NULL;
    int i;

    if (is_empty(q)) {
        return res;
    }

    if (q->front->package.idx > 0) {
        res = (int*) malloc(q->front->package.idx * sizeof(int));
        
        for (i = 0; i < q->front->package.idx; ++i) {
            res[i] = i;
        }
        *size = q->front->package.idx;
        return res;
    }

    queue* tmp = q->front;

    while ((tmp->next != NULL) && (tmp->package.idx + 1 == tmp->next->package.idx)) {
        tmp = tmp->next;
    }

    if (tmp != NULL) {
        int target = tmp->next->package.idx;
        res = (int*) malloc((target - tmp->package.idx - 1) * sizeof(int));

        for (i = tmp->package.idx + 1; i < target; ++i) {
            res[i - tmp->package.idx - 1] = i;
        }
        *size = target - tmp->package.idx - 1;
    }
    return res;
}

pkg top(q_head* q) {
    return q->front->package;
}

void destroy_queue(q_head* q) {
    if (q->front == NULL) {
        free(q);
        return;
    }

    queue* tmp = q->front;
    queue* stalker = tmp;

    while(tmp != NULL) {
        stalker = tmp;
        tmp = tmp->next;

        free(stalker);
    }
    free(q);
}
