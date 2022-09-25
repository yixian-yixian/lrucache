#ifndef FILE_HANDLER_INCLUDED
#define FILE_HANDLER_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h> 
#include "cache.h"
#include "file_node.h"

typedef struct timespec* Time;

size_t readaLine(int fileDescriptor, char **instructions);
size_t readTargetFile(char *fileName, void **address);
int writeTargetFile(char *fileName, void *content, size_t contentSize);

void parseCommand(Cache_T ORG, char *cmd, Time systemTime);
void handlePut(Cache_T ORG, char *contentKey, int maxAge, float entryTime);
void handleGet(Cache_T ORG, char *contentKey, float entryTime);


int deleteTargetFile(char *targetFileName);

#endif