#include "cache.h"

#define NAMELEN 50

Node findOldestStaleinPUT(Cache_T ORG, float currTime);
Node findOldestStaleinGET(Cache_T ORG, float currTime);
Node findNodeinLL(Node head, char *fileName);


/* initializeCache 
 * purpose: initialize a cache of target capacity
 * prereq: capacity is a nonnegative value 
 * return: initialized cache object
 * parameter: 
 *      capacity: requested size of the Cache
*/ 
Cache initializeCache(size_t capacity)
{
    Cache ORG;
    ORG.putSize =0;
    ORG.getSize = 0;
    ORG.cap = capacity;
    ORG.putHead = initNode("PUT HEAD NODE", NULL, 0,0,0);
    ORG.putTail = initNode("PUT TAIL NODE", NULL, 0,0,0);
    ORG.putHead->next = ORG.putTail;
    ORG.putTail->prev = ORG.putHead;
    ORG.getHead = initNode("GET HEAD NODE", NULL, 0,0,0);
    ORG.getTail = initNode("GET TAIL NODE", NULL, 0,0,0);
    ORG.getHead->next = ORG.getTail;
    ORG.getTail->prev = ORG.getHead;
    return ORG;
}

/* cleanCache ()
 * purpose: remove the putList and getList from heap memories 
 * prereq: ORG is an initialized cache with putList and getList 
 *          on heap memories 
 * return: None 
 * parameter: 
 *      ORG: an initialized cache object 
*/
void cleanCache(Cache ORG){
    freeLinkedlist(ORG.putHead);
    freeLinkedlist(ORG.getHead);
}



/* findNode
 * purpose: iterate through both list to identify any Node that has 
 *          the same fileName
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: pointer to the target Node struct; NULL if empty or not found
 * parameter: 
 *         ORG: pointer to the cache object with two list
 *         keyName: target filename that we are looking for
 */
Node findNode(Cache_T ORG, char *keyName)
{
    Node target = NULL;
    /* uninitailized empty cache */
    if (ORG->putSize == 0 && ORG->getSize == 0) return target;
    /* putList is nonempty */
    target = findNodeinLL(ORG->putHead, keyName);
    /* getList is nonempty */
    if (target == NULL) {
        target = findNodeinLL(ORG->getHead, keyName);
    }
    return target;
}



/* findOldestStale
 * purpose: iterate through both lists to identify the oldest stale item
 *          and update the cache for the total number of stale items
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: pointer to the oldest state Node; NULL if none were stale
 * parameter: 
 *         ORG: pointer to the cache object with two list
 *         currTime: current CPU time of the operation 
 * notes: update the total number of stale items record in Cache
 */
Node findOldestStale(Cache_T ORG, float currTime)
{
    Node oldest = findOldestStaleinPUT(ORG, currTime);
    if (oldest == NULL) oldest = findOldestStaleinGET(ORG, currTime);
    return oldest;
}

/* evictCache
 * purpose: following the evicting cache policy to consider which node to evict 
 *          first priority -> remove oldest stale nodes
 *          second priority -> remove oldest nonretrieved node (from putList)
 *          third priority -> remove oldest retrieved node (lru node from getList)
 * prereq: ORG has to be an initialized Cache 
 * return: None 
 * parameter: 
 *      ORG: pointer to an initialized cache object 
 *      currTime: current time in float unit of the operation
*/
void evictCache(Cache_T ORG, float currTime)
{
    Node oldestStale = findOldestStale(ORG, currTime);
    /* remove oldest stale node if the cache is full */
    if (oldestStale != NULL) {
        deleteTargetFile(oldestStale->fileName);
        removeNode(oldestStale);
    } else { /* remove the oldest non-retrieved one first */
        if (ORG->putSize != 0) {
            deleteTargetFile(ORG->putTail->prev->fileName);
            popTail(ORG->putTail);
            ORG->putSize -= 1;
        } else {/* remove LRU if all were retrieved once */
            deleteTargetFile(ORG->getTail->prev->fileName);
            popTail(ORG->getTail);
            ORG->getSize -= 1;
        }
    }
}

/* deleteTargetFile 
 * purpose: remove the file corresponding to the fileNode in the folder
 * prereq: None 
 * return: 0 if successfully removed, not 0 if unsuccessful modification 
 * parameter: 
 *      target: fileNode with fileName that should be removed
*/
int deleteTargetFile(char *targetFileName)
{
    struct stat buf;
    if (stat(targetFileName, &buf) == 0){
        remove(targetFileName);
        return 0;
    } 
    return -1;
}


/* shouldEvit()
 * purpose: check if the provided cache has a full capacity cache 
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: True if Cache is at full capacity, False if not
 * parameter: 
 *         ORG: pointer to the cache object with two list
*/
bool shouldEvict(Cache_T ORG){
    return (ORG->putSize + ORG->getSize >= ORG->cap) ? true : false;
}

/*  * * * * * * * * Local helper functions  * * * * * * * * * * * * */
/* findNodeinLL
 * purpose: iterate through a linkedlist to identify any Node that has
 *          the same fileName
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: pointer to the target Node struct; NULL if not found
 * parameter: 
 *         ORG: pointer to the cache object with two list
 *         keyName: target filename that we are looking for
 */
Node findNodeinLL(Node head, char *fileName)
{
    while(head != NULL){
        if (strcmp(head->fileName, fileName) == 0) break;
        head = head->next;
    }
    return (head != NULL) ? head : NULL;
}


/* isStale()
 * purpose: check if the provided target node has gone stale according
 *          to the parameter currentTime and the record maxAge
 * prereq: target must be an initialized node
 * return: True if the node has gone stale,  False otherwise 
 * parameter: 
 *         currTime: current time in float unit of the operation
 *          target: pointer to the target filenode
*/
bool isStale(float currTime, Node target)
{
    assert(target != NULL);
    float timeElapsed = (currTime - target->entryTime) / pow(10,9);
    if (timeElapsed > target->maxAge) return true;
    return target->maxAge == 0 ? true : false;
}

/* findOldestStaleinPUT
 * purpose: iterate through putlist to identify the oldest stale item
 *          and update the cache for the total number of stale items
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: pointer to the oldest state Node; NULL if none were stale
 * parameter: 
 *         ORG: pointer to the cache object with two list
 *         currTime: current CPU time of the operation 
 * notes: update the total number of stale items record in Cache
 */
Node findOldestStaleinPUT(Cache_T ORG, float currTime)
{
    float maxTimeDiff = 0.0;
    Node curr = ORG->putHead->next;
    Node oldest = NULL;
    while(curr != ORG->putTail){
        if (isStale(currTime, curr)){
            float timeElapsed = (currTime - curr->entryTime) / pow(10,9);
            if (timeElapsed > maxTimeDiff) {
                oldest = curr; 
                maxTimeDiff = timeElapsed;
            }
        }
        curr = curr->next;
    }
    return (oldest != ORG->putHead) ? oldest : NULL;
}

/* findOldestStaleinGET
 * purpose: iterate through putlist to identify the oldest stale item
 *          and update the cache for the total number of stale items
 * prereq: Cache_T must be an address of an initialized Cache struct 
 * return: pointer to the oldest state Node; NULL if none were stale
 * parameter: 
 *         ORG: pointer to the cache object with two list
 *         currTime: current CPU time of the operation 
 * notes: update the total number of stale items record in Cache
 */
Node findOldestStaleinGET(Cache_T ORG, float currTime)
{
    float maxTimeDiff = 0.0;
    Node curr = ORG->getHead->next;
    Node oldest = NULL;
    while(curr != ORG->getTail){
        if (isStale(currTime, curr)){
            float timeElapsed = (currTime - curr->entryTime) / pow(10,9);
            if (timeElapsed > maxTimeDiff) {
                oldest = curr; 
                maxTimeDiff = timeElapsed;
            }
        }
        curr = curr->next;
    }
    return (oldest != ORG->getHead) ? oldest : NULL;
}
