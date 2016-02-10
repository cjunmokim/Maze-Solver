/* ========================================================================== */
/* File: list.h
 *
 * Project name: Maze Runner
 *
 * This file contains the definitions for a doubly-linked list of WebPages.
 *
 */
/* ========================================================================== */
#ifndef LIST_H
#define LIST_H

// ---------------- Prerequisites e.g., Requires "math.h"
#include <stdio.h>                           // printf

// ---------------- Local includes  e.g., "file.h"
#include <math.h>
#include <ctype.h>

#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>                          // strlen
#include <stdlib.h>


// ---------------- Constants

// ---------------- Structures/Types

typedef struct ListNode {
    void* page;                           // the data for a given page (USE DOC NODES)
    struct ListNode *prev;                   // pointer to previous node
    struct ListNode *next;                   // pointer to next node
} ListNode;

typedef struct List {
    ListNode *head;                          // "beginning" of the list
    ListNode *tail;                          // "end" of the list
} List;

typedef struct DocumentNode {
  struct DocumentNode *next;         // pointer to the next member of the list.
  int doc_id;                        // document identifier
  int freq;                          // number of occurrences of the word
} DocumentNode;

typedef struct WordNode {
  struct WordNode *next;            // pointer to the next word (for collisions)
  char *word;                       // the word
  DocumentNode *page;               // pointer to the first element of the page list.
} WordNode;


// ---------------- Public Variables

// ---------------- Prototypes/Macros

/*
* Add to the Linked list in a sorted fashion, based on the the freq of the document node
* Can't be entirely generalized, but serves as a model if want to rank based on another element
*/
int LinkedListAdd(List* myList,void* page);
/*
* Pop the head node from the linked list
*/
void* LinkedListRemove(List *URLList);
/*
* Remove a specific node from a linked list
*/
int RemoveListNode(List *myList,ListNode *myNode);

#endif // LIST_H