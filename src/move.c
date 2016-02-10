/* ========================================================================== */
/* File: move.c
 *
 * Author: Will McConnell, Nick Schwartz, Caleb Junmo Kim
 * Date: 8/21/15
 *
 * Description: This file contains functions to determine the avatars next move.
 *  
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"
#include "maze.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes

/* ========================================================================== */


/* This is the function that handles the primary decision making about the
 * avatar's next move based on the current information about the avatar.
 *
 * It takes into account what "phase" of the maze solving process we are in,
 * either mapping or meeting.
 *
 * Input:
 * (1) phase = what phase of the maze solving we are in
 * (2) maze = the maze array structure
 * (3) last_move = whatever the last move of the avatar was
 * (4) blocked = was the last move of the avatar blocked?
 * (5) pos = the current position of the avatar
 *
 * Output:
 * (1) the direction of the avatar's next move
 */
int make_decision(BlockNode*** maze, int last_move, int blocked, XYPos* pos){
	int x = (int)pos->x;
	int y = (int)pos->y;
	

	// Is it the first move this avatar has made?
	if (last_move == -1){
		// If so, just move them west to start
		return M_WEST;
	}
	// Otherwise, we are mapping and it's NOT the first move
	else{
		// If the avatar's last move was west...
		if (last_move == M_WEST){
			// And they were blocked...
			if (blocked){
				if (maze[x][y]->South == 1 || maze[x][y]->South == -1){ //if south is not blocked, move south
					return M_SOUTH;
				}
				if (maze[x][y]->East == 1 || maze[x][y]->East == -1){ //if south is blocked, and east is not blocked, move east
					return M_EAST;
				}
				return M_NORTH; //if south AND east are blocked, move north
			}
			// Or they were NOT blocked...
			if (maze[x][y]->North == 1 || maze[x][y]->North == -1){ //if north is not blocked, move north
					return M_NORTH;
			}
			if (maze[x][y]->West == 1 || maze[x][y]->West == -1){ //if north is blocked, and west is not blocked, move west
					return M_WEST;
			}
			if (maze[x][y]->South == 1 || maze[x][y]->South == -1){ //if north and west are blocked, and south is not blocked, move south
					return M_SOUTH;
			}
			return M_EAST; //if north AND west AND south are blocked, move east
		}
		// If the avatar's last move was north...
		else if (last_move == M_NORTH){
			// And they were blocked...
			if (blocked){
				if (maze[x][y]->West == 1 || maze[x][y]->West == -1){
					return M_WEST;
				}
				if (maze[x][y]->South == 1 || maze[x][y]->South == -1){
					return M_SOUTH;
				}
				return M_EAST;
			}
			// Or they were NOT blocked...
			if (maze[x][y]->East == 1 || maze[x][y]->East == -1){
					return M_EAST;
			}
			if (maze[x][y]->North == 1 || maze[x][y]->North == -1){
					return M_NORTH;
			}
			if (maze[x][y]->West == 1 || maze[x][y]->West == -1){
					return M_WEST;
			}
			return M_SOUTH;
		}
		// If the avatar's last move was south...
		else if (last_move == M_SOUTH){
			// And they were blocked...
			if (blocked){
				if (maze[x][y]->East == 1 || maze[x][y]->East == -1){
					return M_EAST;
				}
				if (maze[x][y]->North == 1 || maze[x][y]->North == -1){
					return M_NORTH;
				}
				return M_WEST;
			}
			// Or they were NOT blocked...
			if (maze[x][y]->West == 1 || maze[x][y]->West == -1){
					return M_WEST;
			}
			if (maze[x][y]->South == 1 || maze[x][y]->South == -1){
					return M_SOUTH;
			}
			if (maze[x][y]->East == 1 || maze[x][y]->East == -1){
					return M_EAST;
			}
			return M_NORTH;
		}
		// If the avatar's last move was east...
		else if (last_move == M_EAST){
			// And they were blocked...
			if (blocked){
				if (maze[x][y]->North == 1 || maze[x][y]->North == -1){
					return M_NORTH;
				}
				if (maze[x][y]->West == 1 || maze[x][y]->West == -1){
					return M_WEST;
				}
				return M_SOUTH;
			}
			// Or they were NOT blocked...
			if (maze[x][y]->South == 1 || maze[x][y]->South == -1){
					return M_SOUTH;
			}
			if (maze[x][y]->East == 1 || maze[x][y]->East == -1){
					return M_EAST;
			}
			if (maze[x][y]->North == 1 || maze[x][y]->North == -1){
					return M_NORTH;
			}
			return M_WEST;
		}
		// Shouldn't get here
		else{
			return M_NULL_MOVE;
		}
	}
}

/* Returns direction of the next avatar's move.
 *
 * Input:
 * (1) maze = the maze array structure
 * (2) pos = XY position struct.
 * (3) MazeWidth = width of the maze
 * (4) MazeHeight = height of the maze
 * (5) target = target position in the maze
 * (6) my_ID = current avatar's ID
 * (7) TargetAvatar = target avatar's ID
 *
 * Output:
 * (1) the direction of the avatar's next move
 */

int make_decision_meeting(BlockNode*** maze, XYPos* pos, int MazeWidth, int MazeHeight, XYPos* target, int my_ID, int TargetAvatar)
{

	if((!pos) || (!target))
	{
		printf("Null's Passed!\n");
		return -1;
	}
	//Run BFS on computed target, determine next move to move towards that target
	BlockNode *nextMove;
	nextMove=BFS(maze,pos,MazeWidth,MazeHeight,target);

	uint32_t current_x=pos->x;
	uint32_t current_y=pos->y;
	uint32_t target_x=nextMove->myPos->x;
	uint32_t target_y=nextMove->myPos->y;
	// Determine position relative to the target, and determine direction
	// based on this
	if ((current_x==target_x) && (current_y>target_y))
	{
		return M_NORTH;
	}
	else if ((current_x<target_x) && (current_y==target_y))
	{
		return M_EAST;
	}
	else if ((current_x==target_x) && (current_y<target_y))
	{
		return M_SOUTH;	
	}
	else if ((current_x>target_x) && (current_y==target_y))
	{
		return M_WEST;
	}
	else
	{
		//In target block, so don't want to move, wait for other avatars
		return M_NULL_MOVE;
	}
}
