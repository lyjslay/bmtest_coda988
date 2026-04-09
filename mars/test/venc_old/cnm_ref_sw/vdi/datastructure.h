//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2020   CHIPS&MEDIA INC. ALL RIGHTS RESERVED
//
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
//
// The entire notice above must be reproduced on all authorized copies.
//
// Description  :
//-----------------------------------------------------------------------------

#ifndef _DATASTRUCTURE_H
#define _DATASTRUCTURE_H

#include "vdi_osal.h"

/************************************************************************/
/* Queue                                                                */
/************************************************************************/
typedef struct {
    void* data;
} QueueData;

typedef struct {
    Uint8* buffer;
    Uint32          size;
    Uint32          itemSize;
    Uint32          count;
    Uint32          front;
    Uint32          rear;
    osal_mutex_t    lock;
} Queue;

Queue* Queue_Create(
    Uint32    itemCount,
    Uint32    itemSize
);

Queue* Queue_Create_With_Lock(
    Uint32    itemCount,
    Uint32    itemSize
);

void Queue_Destroy(
    Queue* queue
);

/**
 * \brief       Enqueue with deep copy
 */
BOOL Queue_Enqueue(
    Queue* queue,
    void* data
);

/**
 * \brief       Caller has responsibility for releasing the returned data
 */
void* Queue_Dequeue(
    Queue* queue
);

void* Queue_Dequeue_From_Rear(
    Queue* queue
);

void Queue_Flush(
    Queue* queue
);

void* Queue_Peek(
    Queue* queue
);

Uint32 Queue_Get_Cnt(
    Queue* queue
);

/**
 * \brief       @dstQ is NULL, it allocates Queue structure and then copy from @srcQ.
 */
Queue* Queue_Copy(
    Queue* dstQ,
    Queue* srcQ
);

/**
 * \brief       Check the queue is full or not.
 */
BOOL Queue_IsFull(
    Queue* queue
);


/************************************************************************/
/* linked list                                                          */
/************************************************************************/
typedef struct list_node {
    void *data;
    struct list_node *next;
} list_node;

/* linked list */
list_node* list_create(void *data);
void list_destroy(list_node **list);
list_node* list_insert_after(list_node *node, void *data);
BOOL list_remove(list_node **list, list_node *node);
list_node* list_find_node(list_node *list, list_node *node);

#endif /* _DATASTRUCTURE_H_ */

