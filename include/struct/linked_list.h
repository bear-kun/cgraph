#ifndef GRAPH_LINKED_PATH_H
#define GRAPH_LINKED_PATH_H

#include "cgraph/types.h"

typedef struct CGraphLinkedNode_ CGraphLinkedNode;

struct CGraphLinkedNode_ {
  CGraphLinkedNode *next;
  CGraphId id;
};

CGraphLinkedNode *cgraphLinkedInsert(CGraphLinkedNode **predNextPtr,
                                     CGraphId id);
CGraphLinkedNode *cgraphLinkedUnlink(CGraphLinkedNode **predNextPtr);
void cgraphLinkedClear(CGraphLinkedNode **path);

#endif // GRAPH_LINKED_PATH_H
