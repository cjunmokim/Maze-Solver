/* ========================================================================== */
/* File: list.c
 *
 * Project name: Maze Runner
 *
 * Author: Will McConnell
 * Date: 8/2/15
 *
 */
/* ========================================================================== */
#include "list.h"


int LinkedListAdd(List *myList,void* myPage)
{
    if (!myPage)
        return 0;

    //Initialize ListNode
    ListNode *myNode;
    myNode=calloc(1,sizeof(ListNode));
    if (!myNode)
        return 0;

    myNode->page=myPage;

    //Initialize TempNode to scan through list
    ListNode *tempNode;
    
    //Now need to place ListNode in List
    //Add to the end of the list, or the tail
    if (myList->tail == NULL)
    {
        //if first element
        myNode->next=NULL;
        myList->head=myNode;

        myList->tail=myNode;
        myNode->prev=NULL;
        //printf("No prior elements in linked list\n");

    }
    else
    {
        //if added to list
        tempNode=myList->tail;
        //Change next and previous and update tail

        tempNode->next=myNode;
        myNode->prev=tempNode;
        myList->tail=myNode;

        myNode->next=NULL;
        //printf("Prior elements in linked list\n");
    }
    return 1;
}

/*
* Pop the head node from the linked list
*/
void* LinkedListRemove(List *URLList)
{
    //Made void
    void* myPage;

    if (!URLList)
        return NULL;
    //Create temp pointer pointing to head

    ListNode *tempNode;
    tempNode=URLList->head;
    
    //Rewire the linked list
    if (!tempNode->next)
    { //Only one element in the list, so want head and tail to now point to null after popping

        URLList->head=NULL;
        URLList->tail=NULL;
    }
    else if (!tempNode->next->next)
    { //Only two elements in the list, so want head and tail to now point to same element
        URLList->head=URLList->tail;
    }
    else
    { //Multiple elements in the list

        //Change head to new head
        URLList->head=tempNode->next;
    }

    myPage=tempNode->page;

    //Then free the list node (but not the docnode that was associated)

    free(tempNode);

    //Returns page
    return myPage;
}

/*
* Remove a specific node from a linked list
*/
int RemoveListNode(List* myList,ListNode *myNode)
{
    DocumentNode *myDocNode=myNode->page;
    DocumentNode *myHead=myList->head->page;
    DocumentNode *myTail=myList->tail->page;
    if ((myHead->doc_id==myDocNode->doc_id) && (myTail->doc_id==myDocNode->doc_id)) //If only one element in the list (remove head and tail)
    {
        myList->head=NULL;
        myList->tail=NULL;
    }
    else if (myHead->doc_id==myDocNode->doc_id) //If remove head
    {
        ListNode *myForward;
        myForward=myNode->next;
        myList->head=myForward;
        myForward->prev=NULL;

    }
    else if (myTail->doc_id==myDocNode->doc_id) //If remove tail
    {
        ListNode *myPrior;
        myPrior=myNode->prev;
        myList->tail=myPrior;
        myPrior->next=NULL;
    }
    else //If remove node in the middle
    {
        ListNode *myPrior;
        myPrior=myNode->prev;
        ListNode *myForward;
        myForward=myNode->next;
        //Rewire
        myPrior->next=myForward;
        myForward->prev=myPrior;
    }
    free(myNode);
    return 1;
}
