#ifndef QUEUE_H
#define QUEUE_H

#include "cgraph/types.h"

typedef struct {
  CGraphSize capacity, size;
  CGraphSize front, rear;
  CGraphId elems[0];
} CGraphQueue;

CGraphQueue *cgraphQueueCreate(CGraphSize capacity);
void cgraphQueueRelease(CGraphQueue *queue);

static void cgraphQueueClear(CGraphQueue *queue) {
  queue->size = queue->front = queue->rear = 0;
}

static CGraphBool cgraphQueueEmpty(const CGraphQueue *queue) {
  return queue->size == 0;
}

static void cgraphQueuePush(CGraphQueue *queue, const CGraphId item) {
  queue->elems[queue->front] = item;
  if (++queue->front == queue->capacity) queue->front = 0;
  ++queue->size;
}

static CGraphId cgraphQueuePop(CGraphQueue *queue) {
  const CGraphId item = queue->elems[queue->rear];
  if (++queue->rear == queue->capacity) queue->rear = 0;
  --queue->size;
  return item;
}

#endif // QUEUE_H