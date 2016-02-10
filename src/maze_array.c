/* ========================================================================== */
/* File: maze_array.c
 *
 * Author: Will McConnell, Nick Schwartz, Caleb Junmo Kim
 * Date: 8/21/15
 *
 * Description: This file has functions related to building the 2D maze and doing the 
 * 				BFS algorithm on the maze.
 *
 *  
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

// ---------------- Local includes  e.g., "file.h"
#include "maze.h"
#include "amazing.h"
#include "list.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes

/* ========================================================================== */


/*
* Performs BFS
* Input: Takes computed target position and the position of the current avatar
* Output: Returns list of BlockNodes that the avatar needs to traverse to reach the target
*/
BlockNode* BFS(BlockNode*** Maze,XYPos *myPosition,int MazeWidth, int MazeHeight, XYPos *target)
{
	// Reset every location in the maze (set distance -1, parent to -1)
	for (int i = 0; i < MazeWidth; i++)
	{
		for (int j = 0; j < MazeHeight; j++)
		{
			Maze[i][j]->BFSParent = NULL;
			Maze[i][j]->BFSDistance = -1;
		}
	}
	
	// Create the queue
	List *queue;
	queue=calloc(1,sizeof(List)); //Should free after 
	
	// Avatar's coordinates
	int x_coor = myPosition->x;
	int y_coor = myPosition->y;
	
	// Target's coordinates
	int target_x_coor = target->x;
	int target_y_coor = target->y;
	
	// The BlockNode on which the avatar currently lives
	BlockNode *myBlockNode;
	myBlockNode=Maze[x_coor][y_coor];
	myBlockNode->BFSDistance=0;
	//Already at target position
	if ((myBlockNode->myPos->x==target_x_coor) && (myBlockNode->myPos->y==target_y_coor))
	{
		//Return this target position BlockNode
		free(queue);
		return myBlockNode;
	}

	BlockNode *pathBlockNode;
	//Add to List
	LinkedListAdd(queue,myBlockNode);
	//while list isn't empty:
	while (queue->head!=NULL)
	{
		//pop from head
		BlockNode *thisBlockNode=LinkedListRemove(queue);
		int this_x_coor=thisBlockNode->myPos->x;
		int this_y_coor=thisBlockNode->myPos->y;
		int thisDistance=thisBlockNode->BFSDistance;
		//printf("This Node %d,%d \n", this_x_coor,this_y_coor);

		//If popped node is the target
		if ((thisBlockNode->myPos->x==target_x_coor) && (thisBlockNode->myPos->y==target_y_coor))
		{
			pathBlockNode=thisBlockNode; //maybe error if null!!!
			
			//Read up the tree until reach the next move from the current position on the path to the final position
			while (CheckSameXYPos(pathBlockNode->BFSParent->myPos,myBlockNode->myPos)!=1) 
			{
				pathBlockNode=pathBlockNode->BFSParent;
				
			}
			break;	
		}
		//for each node that is adjacent (has a 1)
		if (thisBlockNode->North==1)
		{
			BlockNode *nextBlockNode=Maze[this_x_coor][this_y_coor-1];
			
			//if distance is -1
			if (nextBlockNode->BFSDistance==-1)
			{
				//distance is parent +1
				nextBlockNode->BFSDistance=thisDistance+1;
				//parent of this node is thisBlockNode
				nextBlockNode->BFSParent=thisBlockNode;
				//add to tail
				LinkedListAdd(queue,nextBlockNode);
			}
		}
		if (thisBlockNode->South==1)
		{
			BlockNode *nextBlockNode=Maze[this_x_coor][this_y_coor+1];
			
			//if distance is -1
			if (nextBlockNode->BFSDistance==-1)
			{

				//distance is parent +1
				nextBlockNode->BFSDistance=thisDistance+1;
				//parent of this node is thisBlockNode
				nextBlockNode->BFSParent=thisBlockNode;
				//add to tail
				LinkedListAdd(queue,nextBlockNode);
			}
		}
		if (thisBlockNode->East==1)
		{
			BlockNode *nextBlockNode=Maze[this_x_coor+1][this_y_coor];
			
			//if distance is -1
			if (nextBlockNode->BFSDistance==-1)
			{
				//distance is parent +1
				nextBlockNode->BFSDistance=thisDistance+1;
				//parent of this node is thisBlockNode
				nextBlockNode->BFSParent=thisBlockNode;
				//add to tail
				LinkedListAdd(queue,nextBlockNode);
			}
		}
		if (thisBlockNode->West==1)
		{
			BlockNode *nextBlockNode=Maze[this_x_coor-1][this_y_coor];
			
			//if distance is -1
			if (nextBlockNode->BFSDistance==-1)
			{
				//distance is parent +1
				nextBlockNode->BFSDistance=thisDistance+1;
				//parent of this node is thisBlockNode
				nextBlockNode->BFSParent=thisBlockNode;
				//add to tail
				LinkedListAdd(queue,nextBlockNode);
			}
		}
	}

	if (queue!=NULL)
	{
		ListNode *node;
		while (queue->head != NULL)
		{
			node=queue->head;
			queue->head=queue->head->next;

			free(node);
			node=NULL;
		}
		free(queue);
	}
	return pathBlockNode;
}


/* This function takes some information about the maze and creates the maze
 *
 * Input:
 * (1) width = the width of the maze
 * (2) height = the height of the maze
 * (3) num_avatars = the number of avatars in the maze
 *
 * Output:
 * (1) A pointer to the maze
 */
BlockNode*** InitializeArray(int width, int height, int num_avatars){
	// Some indexing variables
	int i, j, k;

	// Create the maze
	BlockNode*** maze = (BlockNode***)calloc(width, sizeof(BlockNode**));

	// Create another dimension to the maze
	for (i = 0; i < width; i++) {
		maze[i] = (BlockNode**)calloc(height, sizeof(BlockNode*));
	}

	// This BlockNode* will takes the values for each spot in the maze
	BlockNode* myBlockNode;
	
	// For every single spot in the maze...
	for (i = 0; i < width; i++){
		for (j = 0; j < height; j++){
			
			// Allocate space for the maze spot
			myBlockNode = (BlockNode*)calloc(1, sizeof(BlockNode));

			// Assign direction variables
			myBlockNode->North = -1;
			myBlockNode->South = -1;
			myBlockNode->East = -1;
			myBlockNode->West = -1;
			
			// Fill in the walls we KNOW are walls i.e. at the edges of the maze
			if (i == 0)
			{
				myBlockNode->West = 0;
			}
			if (j == 0)
			{
				myBlockNode->North = 0;
			}
			if (i == width-1)
			{
				myBlockNode->East = 0;
			}
			if (j == height-1)
			{
				myBlockNode->South = 0;
			}
			
			// Initialize the array of avatars who have visited
			myBlockNode->AvatarVisited = (int*)calloc(num_avatars, sizeof(int));
			for(k = 0; k < num_avatars; k++){
				myBlockNode->AvatarVisited[k] = 0;
			}
			
			myBlockNode->myPos = (XYPos*)calloc(1, sizeof(XYPos));
			
			//Initialize XYPos
			myBlockNode->myPos->x = i;
			myBlockNode->myPos->y = j;
			// Add this spot to the maze, then repeat for each spot in the maze
			maze[i][j] = myBlockNode;
		}
	}

	// Lastly, return the newly created maze
	return maze;
}


/* This function prints out all the information about a given maze
 *
 * Input:
 * (1) maze = the maze to be printed out
 * (2) width = the width of the maze
 * (3) height = the height of the maze
 * (4) num_avatars = the number of avatars in the maze
 *
 * Output: No return value; prints information about the maze to the console
 */
void PrintMaze(BlockNode*** maze, int width, int height, int num_avatars){
	// Some indexing variables
	int i, j, k;
	
	// For every single spot in the maze...
	for (i=0; i < width; i++){
		for (j=0; j < height; j++){
			// Print out information about that spot in the maze
			printf("Maze coordinates: (%d, %d) -- North: %d -- South: %d -- East: \
%d -- West: %d ----- ", i, j, maze[i][j]->North, maze[i][j]->South, 
			maze[i][j]->East, maze[i][j]->West);
			
			printf("Avatars: ");
			for (k  = 0; k < num_avatars; k++){
				printf("%d ", maze[i][j]->AvatarVisited[k]);
			}
			printf("\n");
		}
	}
}

/*
* Compare XYPos structs.
* Returns 1 if same location, 0 if not, -1 if null. 
*/
int CheckSameXYPos(XYPos *prevPosition,XYPos *currentPosition)
{
	uint32_t prev_x=prevPosition->x;
	uint32_t prev_y=prevPosition->y;
	uint32_t curr_x=currentPosition->x;
	uint32_t curr_y=currentPosition->y;

	if ((prev_x==curr_x) && (prev_y==curr_y))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
* When avatar's initially placed on maze, update the maze to reflect their position
*/
int InitialPlacing(BlockNode*** Maze,XYPos *avatarPosition,int myAvatar)
{
	//Null check
	if (!avatarPosition)
	{
		printf("Error, location is null!\n");
		return EXIT_FAILURE;
	}
	
	uint32_t curr_x=avatarPosition->x;
	uint32_t curr_y=avatarPosition->y;

	Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;

	return EXIT_SUCCESS;
}
/*
* Determine the center needed for BFS search
* Input: Array of the position of each avatar
* Output: XY struct of the centers
*/
XYPos* DetermineCenter(BlockNode*** Maze,XYPos *myAvatarPosArray[],int NumOfAvatars, int MazeWidth, int MazeHeight)
{
	int totalX=0;
	int totalY=0;
	XYPos *myCenter;
	myCenter=calloc(1,sizeof(XYPos));

	for (int i=0;i<NumOfAvatars;i++)
	{
		totalX=totalX+(myAvatarPosArray[i]->x);
		totalY=totalY+(myAvatarPosArray[i]->y);
	}
	int myCenterX=totalX/NumOfAvatars;
	int myCenterY=totalY/NumOfAvatars;
	printf("Determined Center: %d %d\n",myCenterX,myCenterY);

	int testCenterX=0;
	int testCenterY=0;
	int closestDistance=1000000000;


	//Loop through whole maze
	for (int w=0;w<MazeWidth;w++)
	{
		for (int m=0;m<MazeHeight;m++)
		{
			//Check if this node can be visited by all avatars
			int traversedPath=1;
			for (int j=0;j<NumOfAvatars;j++)
			{
				if (Maze[w][m]->AvatarVisited[j]==0)
				{
					traversedPath=0;
				}
			}
			//If node can be visited
			if (traversedPath==1)
			{
				//If closer to center than previously assigned center, then update center
				int thisDistance=abs(myCenterX-w)+abs(myCenterY-m);
				if (thisDistance<closestDistance)
				{
					closestDistance=thisDistance;
					testCenterX=w;
					testCenterY=m;
				}
			}
		}
	}

	myCenter->x=testCenterX;
	myCenter->y=testCenterY;

	printf("MyCenter: %d %d\n",myCenter->x, myCenter->y);
	return myCenter;
}


/* This function determines if the maze has been fully mapped
 *
 * Input:
 * (1) maze = the maze to be checked
 * (2) width = the width of the maze
 * (3) height = the height of the maze
 *
 * Output:
 * (1) 1 if the maze is mapped, 0 if not
 */
int MazeMapped(BlockNode*** maze, int width, int height){
	// Some indexing variables
	int i, j;
	
	// The mapped boolean will be true until otherwise made false
	int mapped = 1;
	
	// For every single spot in the maze...
	for (i=0; i < width; i++){
		for (j=0; j < height; j++){
			// If any of the spots are still unknown, the maze is not mapped
			if (maze[i][j]->East == -1){
				mapped = 0;
			}
			if (maze[i][j]->West == -1){
				mapped = 0;
			}
			if (maze[i][j]->North == -1){
				mapped = 0;
			}
			if (maze[i][j]->South == -1){
				mapped = 0;
			}
		}
	}
	// Return the result
	return mapped;
}

// Function to free the maze.
void freeMaze(BlockNode*** maze, int width, int height) {
	
	// Variable for each node.
	BlockNode* node;
	
	// Loop through each bin of the maze matrix and free the BlockNode.
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			node = maze[i][j];
			
			// Cleanup.
			node->BFSParent = NULL;
			free(node->AvatarVisited);
			free(node->myPos);
			free(node);
		}
		free(maze[i]);
	}
	free(maze);
	maze = NULL;
}


