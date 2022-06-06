#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

typedef struct _Node {
    MemberInfo *data;
    MemberInfo *next;
} Node;

typedef struct _Queue {
    int count;
    Node *front, *rear;
} Queue;

void initQueue(Queue *);
bool isEmpty(Queue *);
void enqueue(Queue *, MemberInfo);
MemberInfo dequeue(Queue *);

#endif