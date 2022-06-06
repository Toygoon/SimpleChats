#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

void initQueue(Queue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

bool isEmpty(Queue *queue) {
    return queue->count == 0;
}

void enqueue(Queue* queue, MemberInfo member) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = &member;
    node->next = NULL;

    if (isEmpty(queue)) {
        queue->front = node;
    } else {
        queue->rear->next = node;
    }
    queue->rear = node;
    queue->count++;
}

MemberInfo dequeue(Queue* queue) {
    Node* node;

    node = queue->front;
    
}