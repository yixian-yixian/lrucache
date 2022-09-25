#ifndef CACHE_INCLUDED
#define CACHE_INCLUDED
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "file_node.h"

typedef struct cache Cache;
typedef Cache* Cache_T;


struct cache {
    size_t putSize;
    size_t getSize;
    size_t cap;
    Node putHead, putTail;
    Node getHead, getTail;
};


Cache initializeCache(size_t capacity);
void cleanCache(Cache ORG);

Node findNode(Cache_T ORG, char *keyName);
void evictCache(Cache_T ORG, float currTime);
void updateNode(Node target, void *content, int maxAge, size_t contentSize, float entryTime);
int deleteTargetFile(char *targetFileName);
bool isStale(float currTime, Node target);
bool shouldEvict(Cache_T ORG);


#endif
