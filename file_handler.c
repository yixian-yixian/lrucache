#include "file_handler.h"

const char *output = "_output";
const char connector = '.';

/* parseCommand 
 * purpose: process the command line string to execute target operation on 
 *          target Cache object
 * prereq: command is either PUT or GET
 * return: None
 * parameter: 
 *      cmd: string consisting of commands 
 */
void parseCommand(Cache_T ORG, char *cmd, Time systemTime)
{
    assert(cmd[0] == 'P' || cmd[0] == 'G');
    int maxAge;
    char *filename, *save, *intermediate; 
    clock_gettime(CLOCK_MONOTONIC, systemTime);
    if (cmd[0] == 'P') { /* PUT command */
        char * save = calloc(strlen(cmd)+1, sizeof(char));
        strcpy(save, cmd);
        filename = strtok(cmd, "PUT: ");
        filename = strtok(filename, "\\");
        filename[strlen(filename)] = '\0';
        intermediate = strchr(save, 'M');
        maxAge = atoi(intermediate += 8);
        free(save);
        handlePut(ORG, filename, maxAge, (float)systemTime->tv_nsec);
    } else { /* GET command */
        filename = cmd + 5;
        handleGet(ORG, filename, (float)systemTime->tv_nsec);
    }
}

/* handlePut
 * purpose: handle a PUT operation to cache
 * preqreq: targetFile is a legal file name
 * return: None 
 * parameter:
 *      targetFile: string representing the file name/path
 *      maxAge: integer represents the time to live of a file
 *      entryTime: CPU seconds elapsed since program started
 */
void handlePut(Cache_T ORG, char *contentKey, int maxAge, float entryTime)
{
    assert(contentKey != NULL);
    void *fileContent = NULL; // free and handled by freeNode
    size_t contentSize = readTargetFile(contentKey, &fileContent);
    /* check if the nodes are present in either list */
    Node node_add = findNode(ORG, contentKey);
    if (node_add != NULL) {
        updateNode(node_add, fileContent, maxAge, contentSize, entryTime);
        movetoHead(ORG->putHead, node_add);
    } else {
        if (shouldEvict(ORG)){ /* check if full cache */
            evictCache(ORG, entryTime);
        }
        /* new insertion for absent filenode */
        node_add = initNode(contentKey, fileContent, maxAge, entryTime, contentSize);
        putNewNode(ORG->putHead, node_add);
        ORG->putSize ++;
    }

}

void handleGet(Cache_T ORG, char *contentKey, float entryTime)
{
    assert(contentKey != NULL);
    Node node_add = findNode(ORG, contentKey);
    if (node_add == NULL) return; /* absent file node retrieval */
    node_add = movetoHead(ORG->getHead, node_add);
    if (node_add->retrieved == false) { /* moving a node from putList to getList */
        setNodeRetrieved(node_add);
        ORG->getSize++;
        ORG->putSize--;
    }
    if (isStale(entryTime, node_add)) { /* retreiving an invalid and stale node */
        node_add->entryTime = entryTime; /* update initialStorage time of the 
        existing node ONLY if it is stale */ 
    }
    /* overwrite and output updated content to the output file */
    writeTargetFile(node_add->fileName, node_add->fileContent, node_add->contentSize);
}


/* readTargetFile 
 * purpose: open the target file and read in the entire file information 
 * prereq: None 
 * return: the total number of bytes read from the target file 
 * parameter: 
 *      fileName: a valid pathname of address string 
*/
size_t readTargetFile(char *fileName, void **address){
    struct stat buffer;
    int fd2 = open(fileName, O_RDONLY);
    if (fd2 < 0) {
        fprintf(stderr, "corrupted file \n");
        return 0;
    }
    /* accessing file size information */
    fstat(fd2, &buffer);
    size_t fileSize = buffer.st_size;
    /* malloc size of filecontent */
    void *fileContent = malloc(sizeof(char) * fileSize);
    /* read in entire file content */
    read(fd2, fileContent,fileSize);
    *address = fileContent;
    return fileSize;
}

/* writeTargetFile 
 * purpose: open the target output file pipe and output all contents 
 *          to the target file source 
 * prereq: None 
 * return: status of the write process: -1 on failure, 0 on success 
 * parameter: 
 *      fileName: a valid pathname of address string 
 *      content: data stored in the cache associated with the target file
 *      contentSize: total number of bytes that should be output into file
*/
int writeTargetFile(char *fileName, void *content, size_t contentSize)
{ 
    char *extension = strchr(fileName, connector);
    char *outputName = calloc(strlen(fileName) + strlen(output) + 1, sizeof(char));
    if(extension == NULL){
        strcpy(outputName, fileName);
        strcat(outputName, output); 
    } else {
        int index = 0;
        while(fileName[index++] != extension[0]){}
        strncpy(outputName, fileName, --index);
        strcat(outputName, output);
        strcat(outputName, extension);
    }
    int fd3 = open(outputName, O_WRONLY | O_CREAT, 0666);
    if (fd3 == -1) return fd3;
    if(write(fd3, content, contentSize) < 0) return -1;
    free(outputName);
    close(fd3);
    return 0;
}


/* readaLine
 * purpose: read the opened file line by line to attain the instructions 
 * prereq: file descriptor is created and opened 
 * return: overall length of the instructions 
 * parameter:
 *      fileDescript: file descriptor address for the opened file
 *      instructions: address of the first bytes of instruction each line
*/
size_t readaLine(int fileDescriptor, char **instructions)
{
    /* initial size of char array is max 50 chars*/
    size_t maxLen = 50;
    size_t numBytes = 0;
    char *lineContents = (char *)malloc(maxLen * sizeof(char));
    assert(lineContents != NULL);

    /* index for current character */
    int unitC = 0;
    int readBytes = read(fileDescriptor, &unitC, 1);
    while(readBytes > 0) {
        if (unitC == '\n' || unitC == EOF){
            break; }
        /* expand space if neccessary */
        if (numBytes > maxLen - 2) { 
            maxLen *= 2;
            lineContents = realloc(lineContents, maxLen);
        }
        lineContents[numBytes++] = (char)unitC;
        readBytes = read(fileDescriptor, &unitC, 1);
    }
    lineContents[numBytes] = '\0';
    if (unitC == EOF) {
        free(lineContents);
        *instructions = NULL;
        return 0;
    }
    *instructions = lineContents;
    return numBytes;
}
