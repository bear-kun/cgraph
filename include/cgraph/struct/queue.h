#ifndef CGRAPH_QUEUE_H
#define CGRAPH_QUEUE_H

#include "cgraph/graph.h"

typedef struct {
  CGraphSize capacity, size;
  CGraphSize front, rear;
  CGraphId elems[];
} CGraphQueue;

CGraphQueue *cgraph_new_queue(CGraphSize capacity);
void cgraph_delete_queue(CGraphQueue *queue);

static void cgraph_queue_clear(CGraphQueue *queue) {
  queue->size = queue->front = queue->rear = 0;
}

static CGraphBool cgraph_queue_empty(const CGraphQueue *queue) {
  return queue->size == 0;
}

static void cgraph_queue_push(CGraphQueue *queue, const CGraphId item) {
  queue->elems[queue->front] = item;
  if (++queue->front == queue->capacity) queue->front = 0;
  ++queue->size;
}

static CGraphId cgraph_queue_pop(CGraphQueue *queue) {
  const CGraphId item = queue->elems[queue->rear];
  if (++queue->rear == queue->capacity) queue->rear = 0;
  --queue->size;
  return item;
}

#endif // CGRAPH_QUEUE_H