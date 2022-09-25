# Networks a0: Least Recently Used Cache 

## driver function: main.c
## data structure:
- indivdual file nodes: file_node.h
    - stored its contentKey and contentNodes 
    - store the entryTime (only update with PUT action)   
- overall cache structure: cache.h
    - there is information about size of putList and getList
    - file nodes are connected as linkedlist in putList and getList 
- process commands and input/output stream of files: file_handler.h
    - send corresponding information to cache to handle 
    - operate on cache structure when there is an order change
      due to update by retrieval
