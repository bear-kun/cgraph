#include "cgraph/struct/queue.h"
#include <stdlib.h>

CGraphQueue *cgraph_new_queue(const CGraphSize capacity) {
  CGraphQueue *queue = malloc(sizeof(CGraphQueue) + capacity * sizeof(CGraphId));
  queue->capacity = capacity;
  queue->size = queue->front = queue->rear = 0;
  return queue;
}

void cgraph_delete_queue(CGraphQueue *queue) { free(queue); }