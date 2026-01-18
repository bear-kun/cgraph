#include "struct/queue.h"
#include <stdlib.h>

CGraphQueue *cgraphQueueCreate(const CGraphSize capacity) {
  CGraphQueue *queue =
      malloc(sizeof(CGraphQueue) + capacity * sizeof(CGraphId));
  queue->capacity = capacity;
  queue->size = queue->front = queue->rear = 0;
  return queue;
}

void cgraphQueueRelease(CGraphQueue *queue) { free(queue); }