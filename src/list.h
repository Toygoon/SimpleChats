#ifndef LIST_H
#define LIST_H

#include "common.h"

typedef struct _Node {
    MemberInfo *data;
    MemberInfo *next;
} Node;

int size;

#endif