#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h> 

#include "cache.h"
#include "file_handler.h"
#include "file_node.h"


int main(int argc, char *argv[])
{
    if (argc <= 2){
        fprintf(stderr, "Insufficient argument; please follow format \n\
        ./a.out <text file name> <cache size> \n");
        exit(1);
    }

    /* track system time since clock begins */
    struct timespec trackTime = {0, 0}; 
    
    /* open files here */
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0){
        fprintf(stderr, "error from opening \n");
        perror("c1");
        exit(1);
    }

    /* initialize Cache structure */
    char *totalSize = argv[2];
    Cache target = initializeCache(atoi(totalSize));

    /* read cmd file to process command */
    char *command = NULL;
    int status = readaLine(fd1, &command);
    while (status != 0){ /* not reaching the eof */
        parseCommand(&target, command, &trackTime);
        free(command); /* free ptr for next iteration */
        status = readaLine(fd1, &command);
    }
    free(command);
    cleanCache(target);
    /* close file here */
    if(close(fd1) < 0){
        fprintf(stderr, "error from closing \n");
        exit(1);
    } 
    return 0;
}

