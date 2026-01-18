#include "struct/linked_list.h"
#include <stdlib.h>

CGraphLinkedNode *cgraphLinkedInsert(CGraphLinkedNode **predNextPtr,
                                     const CGraphId id) {
  CGraphLinkedNode *const path = malloc(sizeof(CGraphLinkedNode));
  path->id = id;
  path->next = *predNextPtr;
  *predNextPtr = path;
  return path;
}

CGraphLinkedNode *cgraphLinkedUnlink(CGraphLinkedNode **predNextPtr) {
  CGraphLinkedNode *const path = *predNextPtr;
  *predNextPtr = path->next;
  return path;
}

void cgraphLinkedClear(CGraphLinkedNode **path) {
  for (CGraphLinkedNode *node = *path, *next; node; node = next) {
    next = node->next;
    free(node);
  }
  *path = NULL;
}