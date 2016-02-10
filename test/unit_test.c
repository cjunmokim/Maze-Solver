/* ========================================================================== */
/* File: unit_test.c
 *
 * Project name: CS50 Project
 * Component name: Maze
 *
 * Runs unit tests
 */
/* ========================================================================== */


// each test should start by setting the result count to zero

#define START_TEST_CASE  int rs=0

// check a condition and if false print the test condition failed
// e.g., SHOULD_BE(dict->start == NULL)

#define SHOULD_BE(x) if (!(x))  {rs=rs+1; \
    printf("Line %d Fails\n", __LINE__); \
  }

// return the result count at the end of a test

#define END_TEST_CASE return rs

#include "../src/maze.h"
#include "../src/amazing.h"

//
// A general macro for running a test.
// For example:
//
//  RUN_TEST(TestDAdd1, "DAdd Test case 1");
//
// translates to:
//
// if (!TestDAdd1()) {
//     printf("Test %s passed\n","DAdd Test case 1");
// } else {
//     printf("Test %s failed\n", "DAdd Test case 1");
//     cnt = cnt +1;
// }
//

#define RUN_TEST(x, y) if (!x()) {              \
    printf("Test %s passed\n", y);              \
} else {                                        \
    printf("Test %s failed\n", y);              \
    cnt = cnt + 1;                              \
}

int UpdateMazeOnMove(BlockNode*** Maze,XYPos *prevPosition,XYPos *currentPosition,int myAvatar,int myDirection, int MazeWidth, int MazeHeight,int NumOfAvatars);
int BFS_TEST_CASE();
int INIT_TEST_CASE();
int CHECK_SAME_TEST_CASE();
int CHECK_DIFF_TEST_CASE();
int INITIAL_PLACING_TEST_CASE();
int INITIAL_PLACING_TEST_CASE_FAIL();
int MAKE_DECISION_TEST();
int MAKE_DECISION_TEST_1();
int MAKE_DECISION__MEETING_TEST();
int UPDATE_TEST();
int UPDATE_TEST_1();

int goal_x = -1;
int goal_y = -1;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t graphics_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t maze_mutex = PTHREAD_MUTEX_INITIALIZER;

int PathFound = 0;
XYPos* TargetPosition;
int DoOnce = 0;
int TargetAvatar;

int main(int argc, char* argv[])
{
	if (argc!=1)
	{
		printf("No Parameters Please!\n");
		return 1;
	}

	int cnt=0;

	RUN_TEST(BFS_TEST_CASE,"Test Case 1");
	RUN_TEST(INIT_TEST_CASE,"Test Case 2");
	RUN_TEST(CHECK_SAME_TEST_CASE,"Test Case 3");
	RUN_TEST(CHECK_DIFF_TEST_CASE,"Test Case 4");
	RUN_TEST(INITIAL_PLACING_TEST_CASE,"Test Case 5");
	RUN_TEST(INITIAL_PLACING_TEST_CASE_FAIL,"Test Case 6");
	RUN_TEST(MAKE_DECISION_TEST,"Test Case 7");
	RUN_TEST(MAKE_DECISION_TEST_1,"Test Case 8");
	RUN_TEST(MAKE_DECISION__MEETING_TEST,"Test Case 9");
	RUN_TEST(UPDATE_TEST,"Test Case 10");
	RUN_TEST(UPDATE_TEST_1,"Test Case 11");

	if (!cnt)
	{
		printf("All Passed!\n");
		return 0;
	}
	else
	{
		printf("Some Fails\n");
		return 1;
	}
	return 1;
}

//Functions from maze_array.c
/*
* Tests BFS
*/
int BFS_TEST_CASE()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	XYPos *myPosition;
	myPosition=calloc(1,sizeof(XYPos));
	myPosition->x=1;
	myPosition->y=1;
	XYPos *target;
	target=calloc(1,sizeof(XYPos));
	target->x=1;
	target->y=1;
	BlockNode*** Maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);

	Maze[1][1]->AvatarVisited[0]=1;
	Maze[1][1]->AvatarVisited[1]=1;

	int x_coor = myPosition->x;
	int y_coor = myPosition->y;
	// Target's coordinates
	int target_x_coor = target->x;
	int target_y_coor = target->y;
	
	BlockNode *variable;
	variable=BFS(Maze,myPosition,MazeWidth,MazeHeight,target);

	SHOULD_BE(variable->myPos->x==x_coor);
	free(myPosition);
	free(target);
	freeMaze(Maze,MazeWidth,MazeHeight);
	END_TEST_CASE;
}
/*
* Tests initialize maze function
*/
int INIT_TEST_CASE()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	XYPos *myPosition;
	myPosition=calloc(1,sizeof(XYPos));
	myPosition->x=0;
	myPosition->y=0;
	XYPos *target;
	target=calloc(1,sizeof(XYPos));
	target->x=0;
	target->y=2;
	BlockNode*** variable = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	free(myPosition);
	free(target);
	freeMaze(variable,MazeWidth,MazeHeight);
	SHOULD_BE(variable!=NULL);
	END_TEST_CASE;
}
/*
* Tests check if same position in maze function
*/
int CHECK_SAME_TEST_CASE()
{
	START_TEST_CASE;
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=0;
	prevPosition->y=0;
	XYPos *currentPosition;
	currentPosition=calloc(1,sizeof(XYPos));
	currentPosition->x=0;
	currentPosition->y=2;

	int variable=CheckSameXYPos(prevPosition,currentPosition);
	SHOULD_BE(variable==0);
	free(prevPosition);
	free(currentPosition);
	END_TEST_CASE;
}
/*
* Tests check if same position in maze function (for different positions)
*/
int CHECK_DIFF_TEST_CASE()
{
	START_TEST_CASE;
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=0;
	prevPosition->y=2;
	XYPos *currentPosition;
	currentPosition=calloc(1,sizeof(XYPos));
	currentPosition->x=0;
	currentPosition->y=2;

	int variable=CheckSameXYPos(prevPosition,currentPosition);
	SHOULD_BE(variable==1);
	free(prevPosition);
	free(currentPosition);
	END_TEST_CASE;
}
/*
* Tests initial placing of avatars in maze function
*/
int INITIAL_PLACING_TEST_CASE()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=0;
	prevPosition->y=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	int variable=InitialPlacing(maze,prevPosition,1);
	SHOULD_BE(variable==EXIT_SUCCESS);
	free(prevPosition);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}
/*
*Tests initial placing of avatars in maze function (failure)
*/
int INITIAL_PLACING_TEST_CASE_FAIL()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	XYPos *prevPosition;
	prevPosition=NULL;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	int variable=InitialPlacing(maze,prevPosition,1);
	SHOULD_BE(variable==EXIT_FAILURE);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}

//Functions from  move.c
/*
* Make decision to move based on RHR
*/
int MAKE_DECISION_TEST()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=2;
	prevPosition->y=2;
	int variable=make_decision( maze, -1, 0, prevPosition);
	SHOULD_BE(variable==M_WEST);
	free(prevPosition);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}
/*
* Make decision to move based on RHR
*/
int MAKE_DECISION_TEST_1()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=2;
	prevPosition->y=2;
	int variable=make_decision( maze, M_WEST, 1, prevPosition);
	SHOULD_BE(variable==M_SOUTH);
	free(prevPosition);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}
/*
* Make decision to move based on BFS
*/
int MAKE_DECISION__MEETING_TEST()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=2;
	prevPosition->y=2;
	XYPos *target;
	target=NULL;
	int variable=make_decision_meeting(maze, prevPosition,MazeWidth,MazeHeight,target,1,0);
	SHOULD_BE(variable==-1);
	free(prevPosition);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}

//Functions from maze.c
/*
*Update maze function on avatar's move
*/
int UPDATE_TEST()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	XYPos *prevPosition;
	prevPosition=calloc(1,sizeof(XYPos));
	prevPosition->x=2;
	prevPosition->y=2;
	XYPos *current;
	current=calloc(1,sizeof(XYPos));
	current->x=2;
	current->y=1;
	int variable=UpdateMazeOnMove(maze,prevPosition,current,1,M_SOUTH,5,5,2);
	SHOULD_BE(variable==1);
	free(prevPosition);
	free(current);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}
/*
* Update maze function on avatar's move
*/
int UPDATE_TEST_1()
{
	START_TEST_CASE;
	int MazeWidth=5;
	int MazeHeight=5;
	int NumOfAvatars=2;
	BlockNode*** maze = InitializeArray(MazeWidth, MazeHeight, NumOfAvatars);
	XYPos *prevPosition;
	prevPosition=NULL;
	XYPos *current;
	current=calloc(1,sizeof(XYPos));
	current->x=2;
	current->y=1;
	int variable=UpdateMazeOnMove(maze,prevPosition,current,1,M_SOUTH,5,5,2);
	SHOULD_BE(variable==0);
	free(prevPosition);
	free(current);
	freeMaze(maze,5,5);
	END_TEST_CASE;
}

/*
* Update the maze based on avatar's move
*/
int UpdateMazeOnMove(BlockNode*** Maze,XYPos *prevPosition,XYPos *currentPosition,int myAvatar,int myDirection, int MazeWidth, int MazeHeight,int NumOfAvatars)
{
	pthread_mutex_lock(&maze_mutex);
	//Null check
	if ((!prevPosition) || (!currentPosition))
	{
		printf("Error, location is null!\n");
		return 0;
	}
	uint32_t prev_x=prevPosition->x;
	uint32_t prev_y=prevPosition->y;
	uint32_t curr_x=currentPosition->x;
	uint32_t curr_y=currentPosition->y;

	if (CheckSameXYPos(prevPosition,currentPosition)==1)
	{
		//printf("Didn't Move!\n");
		//Update that BlockNode's direction as a wall
		if (myDirection==0)
		{
			Maze[prev_x][prev_y]->West=0;
			if (prev_x > 0){
				Maze[prev_x - 1][prev_y]->East=0;
			}
		}
		if (myDirection==1)
		{
			Maze[prev_x][prev_y]->North=0;
			if (prev_y > 0){
				Maze[prev_x][prev_y - 1]->South=0;
			}
		}
		if (myDirection==2)
		{
			Maze[prev_x][prev_y]->South=0;
			if (prev_y < MazeHeight - 1){
				Maze[prev_x][prev_y + 1]->North=0;
			}
		}
		if (myDirection==3)
		{
			Maze[prev_x][prev_y]->East=0;
			if (prev_x < MazeWidth - 1){
				Maze[prev_x + 1][prev_y]->West=0;
			}
		}
	}
	else
	{
		//printf("Did move!\n");

		//Need to set previous position's as having a possible move, 
		//and then update new locations'BlockNode AvatarVisited Array 
		if ((prev_x==curr_x) && (prev_y>curr_y))
		{
			//printf("Moved North!\n");
			Maze[prev_x][prev_y]->North=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->South=1;
			
		}
		else if ((prev_x<curr_x) && (prev_y==curr_y))
		{
			//printf("Moved East!\n");
			Maze[prev_x][prev_y]->East=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->West=1;
		}
		else if ((prev_x==curr_x) && (prev_y<curr_y))
		{
			//printf("Moved South!\n");
			Maze[prev_x][prev_y]->South=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->North=1;
		}
		else if ((prev_x>curr_x) && (prev_y==curr_y))
		{
			//printf("Moved West!\n");
			Maze[prev_x][prev_y]->West=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->East=1;
		}
		else
		{
			printf("Error!\n");
			return 0;
		}


		//Need to check if location moved to has been visited by others and update AvatarVisited Array
		//For each avatar
		for (int i=0;i<NumOfAvatars;i++)
		{
			if ((Maze[curr_x][curr_y]->AvatarVisited[i]==1)&&(i!=myAvatar)) //if visited by avatars other than your own
			{
				//Loop through maze and update AvatarVisitedArray
				for (int w=0;w<MazeWidth;w++)
				{
					for (int m=0;m<MazeHeight;m++)
					{
						//Do two things
						//Set everywhere in the maze that other avatar had been to as being able to be visited by our avatar
						if (Maze[w][m]->AvatarVisited[i]==1)
						{
							Maze[w][m]->AvatarVisited[myAvatar]=1;
						}
						//Set everywhere in the maze that our avatar had been to as being able to be visited by other avatar
						if (Maze[w][m]->AvatarVisited[myAvatar]==1)
						{
							Maze[w][m]->AvatarVisited[i]=1;
						}
					}
				}

			}
		}

		//Check if AvatarVisited Array at this new location can be visited by all the avatars
		int traversedPath=1;
		for (int j=0;j<NumOfAvatars;j++)
		{
			if (Maze[curr_x][curr_y]->AvatarVisited[j]==0)
			{
				traversedPath=0;
			}
		}
		//If node can be visited by all visitor, now throw flag for BFS
		if (traversedPath==1)
		{
			//This is the flag thrown
			PathFound=1;
		}


	}
	pthread_mutex_unlock(&maze_mutex);
	return 1;
}



