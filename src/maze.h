/* ========================================================================== */
/* File: maze.h
 *
 * Project name: Maze Runner
 *
 * Contains function prototypes and some constants for the maze runner program
 */
/* ========================================================================== */

// Includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "amazing.h"
#include "list.h"

// Some handy constants
#define MAX_LINE_LENGTH 500
#define MAX_HEADER_LENGTH 5000
#define IP_ADDRESS_LENGTH 100

// A global to track how many threads are still alive and running
extern int running_threads;
extern pthread_mutex_t maze_mutex;
extern pthread_mutex_t log_mutex;
extern pthread_mutex_t graphics_mutex;

// Data structures

// A BlockNode represents a spot in the maze; it has information about each of
// the ways an avatar could possible move. It also has a list of which avatars
// have visited, and a position.
typedef struct BlockNode {
	int North; //-1 if unexplored, 0 if wall, 1 if possible move
    int South; //-1 if unexplored, 0 if wall, 1 if possible move
    int West; //-1 if unexplored, 0 if wall, 1 if possible move
    int East; //-1 if unexplored, 0 if wall, 1 if possible move
    int* AvatarVisited; //List of whether or not avatar has visited
    struct BlockNode *BFSParent;
    int BFSDistance;
    XYPos *myPos;
} BlockNode;

// A "boxed package" of parameters for the avatar to receive
typedef struct AvatarParameter {
	int ID;
	int num_avatars;
	int difficulty;
	char* IP_address;
	uint32_t port;
	char* log_file;
	int sockfd;
	int width;
	int height;
	BlockNode*** maze;
	XYPos** graphics_spots;
} AvatarParameter;

// Function Prototypes

// All function headers for these functions can be found in the respective .c files

// Prototypes for maze_array.c
BlockNode*** InitializeArray(int MazeWidth, int MazeHeight, int NumOfAvatars);
void PrintMaze(BlockNode*** Maze,int MazeWidth, int MazeHeight, int NumOfAvatars);
int CheckSameXYPos(XYPos *prevPosition,XYPos *currentPosition);
int InitialPlacing(BlockNode*** Maze,XYPos *avatarPosition,int myAvatar);
XYPos* DetermineCenter(BlockNode*** Maze,XYPos *myAvatarPosArray[], int NumOfAvatars,int MazeWidth, int MazeHeight);
BlockNode* BFS(BlockNode*** Maze,XYPos *myPosition,int MazeWidth, int MazeHeight, XYPos *target);
int MazeMapped(BlockNode*** maze, int width, int height);
void freeMaze(BlockNode*** maze, int width, int height);

// Prototypes for maze.c
int avatar_run(uint32_t ID, int Difficulty, char *address, uint32_t MazePort, char *file, int nAvatars, int width, int height, BlockNode*** maze, XYPos** graphics_spots);
int write_a_line_to_file(char* file_name, char* line);
char* get_line_information(AvatarParameter* params);
void upload_graphics_info(XYPos** spots, XYPos* spot, int ID);
void maze_graphics(XYPos** graphics_spots, int width, int height, int num_avatars, int turn_count);
int UpdateMazeOnMove(BlockNode*** Maze,XYPos *prevPosition,XYPos *currentPosition,int myAvatar,int myDirection, int MazeWidth, int MazeHeight,int NumOfAvatars);
void FreeAvatarParam(AvatarParameter *param);

// Prototypes for move.c
int make_decision(BlockNode*** maze, int last_move, int blocked, XYPos* pos);
int make_decision_meeting(BlockNode*** maze, XYPos* pos, int MazeWidth, int MazeHeight, XYPos* target, int my_ID, int TargetAvatar);
