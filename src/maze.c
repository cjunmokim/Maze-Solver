/* ========================================================================== */
/* File: maze.c
 *
 * Author: Will McConnell, Nick Schwartz, Caleb Junmo Kim
 * Date: 8/21/15
 *
 * Description: This file has functions to create threads for each avatar and solve the maze. 
 *
 *  
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

// ---------------- Local includes  e.g., "file.h"
#include "maze.h"
#include "amazing.h"

// ---------------- Constant definitions
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t graphics_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t maze_mutex = PTHREAD_MUTEX_INITIALIZER;

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables
int PathFound = 0;
XYPos* TargetPosition;
int DoOnce = 0;
int TargetAvatar;

// ---------------- Private prototypes

/* ========================================================================== */


/* An avatar thread
 *
 * Input:
 * (1) input = a bundle of parameters passed in as a struct
 *
 * Output: None.
 */
void* avatar(void* input){

	// Cast the input struct pointer to the correct type
	AvatarParameter* params = (AvatarParameter*)input;
	
	// Assign all of the avatars variables
	uint32_t my_ID = params->ID;
	int difficulty = params->difficulty;
	char* IP_address = params->IP_address;
	uint32_t port = params->port;
	char* log_file = params->log_file;
	int sockfd = params->sockfd;
	int num_avatars = params->num_avatars;
	int width = params->width;
	int height = params->height;
	BlockNode*** the_maze = params->maze;
	XYPos** graphics_spots = params->graphics_spots;
	
	// Position variables for the avatar
 	XYPos* my_pos = (XYPos*)calloc(1, sizeof(XYPos));
 	XYPos* old_pos = (XYPos*)calloc(1, sizeof(XYPos));
	
	// line will be used to print out messages to the log
	char *line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));

	// Will hold main avatar info and be printed to the file
	char* avatar_header = get_line_information(params);
	
	write_a_line_to_file(log_file, avatar_header);
	free(avatar_header); avatar_header = NULL;
	
	// Send AM_AVATAR_READY to server
	AM_Message ready_msg;
	memset(&ready_msg, 0, sizeof(AM_Message));
	ready_msg.type = htonl(AM_AVATAR_READY);
	ready_msg.avatar_ready.AvatarId = htonl(my_ID);
	
	// Send the message.
	send(sockfd, &ready_msg, sizeof(ready_msg), 0);
	
	// Waiting for server response...
	AM_Message rec_msg;

	// Receive the AM_AVATAR_TURN message
	if (recv(sockfd, &rec_msg, sizeof(rec_msg), 0) < 0) {
		// If the message was not received, print an error and exit
		sprintf(line, "[avatar #%d] Error receiving initial TURN message\n", my_ID);
		write_a_line_to_file(log_file, line);
		free(line); line = NULL;
		
		// Cleanup.
		FreeAvatarParam(params);
 		free(my_pos);
 		free(old_pos);
		
		running_threads--;
		return NULL;
	}
	
	// Is it an AM_AVATAR_TURN message?
	if (ntohl(rec_msg.type) != AM_AVATAR_TURN) {
		// If not, print an error and exit
		sprintf(line, "[avatar #%d] ERROR: Initial message was not a TURN message\n", my_ID);
		write_a_line_to_file(log_file, line);
		free(line); line = NULL;
		
		// Cleanup.
		FreeAvatarParam(params);
		free(my_pos);
 		free(old_pos);
		
		running_threads--;
		return NULL;
	}
	
	char* initial_messages = (char*)calloc(4*MAX_LINE_LENGTH, sizeof(char));
	
	// Made connection
	sprintf(initial_messages, "[avatar #%d] Successfully connected with the server and received the first TURN message\n", my_ID);
	
	// Variables about the avatar's position and about whose turn it currently is
	uint32_t turn;
	
	// Assign these variables from the first TURN message
	turn = ntohl(rec_msg.avatar_turn.TurnId);
	my_pos->x = ntohl(rec_msg.avatar_turn.Pos[my_ID].x);
	my_pos->y = ntohl(rec_msg.avatar_turn.Pos[my_ID].y);
	
	// Initial variables
	sprintf(line, "[avatar #%d] First Turn ID (should be zero): %u\n[avatar #%d] Initial Coordinates: x=%u y=%u\n\n", my_ID, turn, my_ID, my_pos->x, my_pos->y);
	strcat(initial_messages, line);
	free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	initial_messages = realloc(initial_messages, strlen(initial_messages) + 1);
	write_a_line_to_file(log_file, initial_messages);
	free(initial_messages); initial_messages = NULL;
	
	// Sleep for one second before the solving begins so that all initial
	// info has a chance to be written to the log
	sleep(1);
	
	// A variable to track whether the game has ended
	int game_over = 0;
	
	// The move message that will be sent each turn
	AM_Message mv_msg;
	memset(&mv_msg, 0, sizeof(AM_Message));
	
	// These variables will hold info about the avatar's moves
	int move = -1;
	int last_move = -1;
	int blocked = 0;
	
	// Track the number of turns taken
	int turn_counter = 0;
	
	// Put the avatar into the maze initially
	InitialPlacing(the_maze, my_pos, my_ID);
	
	// As long as the game hasn't ended...
	while(!game_over){
		
		// Print the coordinates of the avatar
		sprintf(line, "[avatar #%d] Turn ID: %u\n[avatar #%d] Starting coordinates this turn: x=%u y=%u\n\n", my_ID, turn, my_ID, my_pos->x, my_pos->y);
		write_a_line_to_file(log_file, line);
		free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));

		// Upload current position to the graphics list of positions
		upload_graphics_info(graphics_spots, my_pos, my_ID);
		if (turn == my_ID){
			// Draw graphics based on everybody's current position
			maze_graphics(graphics_spots, width, height, num_avatars, turn_counter);
		}
		
		// If it's my turn
		if (turn == my_ID) {
			
			if (PathFound==0){
				//In the mapping phase
				move = make_decision(the_maze, last_move, blocked, my_pos);
			}
			else if (PathFound==1){

				//Placeholder. Need more sophisticated way of determining target.
				//As of now just make one of the avatars the target. 
				if (DoOnce==0){
					//Set the global variable
					TargetPosition=calloc(1,sizeof(XYPos)); //Need to free once game is over.
					TargetPosition->x=my_pos->x;
					TargetPosition->y=my_pos->y;
					TargetAvatar=my_ID;
					DoOnce=1;
				}

				
				//In the meeting phase
				move = make_decision_meeting(the_maze,my_pos,width,height,TargetPosition,my_ID,TargetAvatar);
			}
			// Create mv_msg after move is computed
			mv_msg.type = htonl(AM_AVATAR_MOVE);
			mv_msg.avatar_move.AvatarId = htonl(my_ID);
			mv_msg.avatar_move.Direction = htonl(move);
		
			// Send the move
			send(sockfd, &mv_msg, sizeof(mv_msg), 0);
		}
		
		// Receive the AM_AVATAR_TURN message
		AM_Message new_msg;
		
		// If there was an error with the TURN message
		if (recv(sockfd, &new_msg, sizeof(new_msg), 0) < 0) {
			if (my_ID == 0){
				sprintf(line, "ERROR: there was a problem with the TURN message\n");
				write_a_line_to_file(log_file, line);
			}
			free(line); line = NULL;
			
			// Cleanup local
			FreeAvatarParam(params);
			free(my_pos);
			free(old_pos);
		
			running_threads--;
			return NULL;
		}
		
		// Have we solved the maze?
		if (ntohl(new_msg.type) == AM_MAZE_SOLVED){
			if (my_ID == 0){
				sleep(1);
				sprintf(line, "WINNER! nAvatars: %u Difficulty: %u nMoves: %u Hash: %u\n\n", ntohl(new_msg.maze_solved.nAvatars), ntohl(new_msg.maze_solved.Difficulty), ntohl(new_msg.maze_solved.nMoves), ntohl(new_msg.maze_solved.Hash));
				
				write_a_line_to_file(log_file, line);
				free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
			}
			break;
		}
		// Too many moves?
		else if (ntohl(new_msg.type) == AM_TOO_MANY_MOVES){
			if (my_ID == 0){
				sprintf(line, "ERROR: too many moves; lost\n");
				write_a_line_to_file(log_file, line);
			}
			free(line); line = NULL;
			
			// Cleanup local
			FreeAvatarParam(params);
			free(my_pos);
			free(old_pos);
			
			running_threads--;
			return NULL;
		}
		// Is the message type neither a SOLVED message nor a TURN nor a TOO_MANY_MOVES message?
		else if(ntohl(new_msg.type) != AM_AVATAR_TURN){
			if (my_ID == 0){
				sprintf(line, "ERROR: server returned an error: %u\n", ntohl(new_msg.type));
				write_a_line_to_file(log_file, line);
			}
			free(line); line = NULL;
			
			// Cleanup local
			FreeAvatarParam(params);
			free(my_pos);
			free(old_pos);
			
			running_threads--;
			return NULL;
		}

		// Update my old coordinates
		old_pos->x = my_pos->x;
		old_pos->y = my_pos->y;
		// Get new coordinates
		my_pos->x = ntohl(new_msg.avatar_turn.Pos[my_ID].x);
		my_pos->y = ntohl(new_msg.avatar_turn.Pos[my_ID].y);
		
		// If I just moved...
		if (my_ID == turn){
			// Update the maze with my new info
			UpdateMazeOnMove(the_maze, old_pos, my_pos, my_ID, move, width, height, num_avatars);
			
			// Determine whether or not I was just blocked
			if (old_pos->x == my_pos->x && old_pos->y == my_pos->y){
				blocked = 1;
			}
			else{
				blocked = 0;
			}
		}
		
		// What is the new TURN_ID?
		turn = ntohl(new_msg.avatar_turn.TurnId);

		// Update the last move
		last_move = move;
		turn_counter++;
		
	}
	
	// Cleanup.
	free(line); line = NULL;
	FreeAvatarParam(params);
	free(my_pos);
	free(old_pos); old_pos = NULL;
	if (my_ID == 0) {
		free(TargetPosition);
	}
	
	// Once this thread is done, there is one less thread still alive
	running_threads--;
	
	// Done!
	return NULL;
}


/* Starts up each avatar thread
 *
 * Input:
 * (1) ID = the ID of the avatar about to be started
 * (2) difficulty = the difficulty of the maze being run
 * (3) address = the IP address of the server
 * (4) MazePort = the port of the maze on the server
 * (5) file = the name of the log file
 * (6) nAvatars = the total number of avatars in the maze
 *
 * Output:
 * (1) exit status
 */
int avatar_run(uint32_t ID, int difficulty, char *address, uint32_t MazePort, char *file, int nAvatars, int width, int height, BlockNode*** maze, XYPos** graphics_spots) {
	// Declare pthread-related variables.
	pthread_t thread;
	int threadret;
	pthread_attr_t attr;
	
	threadret = pthread_attr_init(&attr);
	threadret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	// Declare socket variables.
	int sockfd;
	struct sockaddr_in servaddr;
	
	// Can pass only a single struct to the avatars that are created
	AvatarParameter* current_params;

	// Assign all the parameters to this struct
	current_params = (AvatarParameter*)calloc(1, sizeof(AvatarParameter));
	current_params->ID = ID;
	current_params->difficulty = difficulty;
	current_params->IP_address = (char*)calloc(strlen(address) + 1, sizeof(char));
	strcpy(current_params->IP_address, address);
	current_params->port = MazePort;
	current_params->log_file = (char*)calloc(strlen(file) + 1, sizeof(char));
	strcpy(current_params->log_file, file);
	current_params->num_avatars = nAvatars;
	current_params->width = width;
	current_params->height = height;
	current_params->maze = maze;
	current_params->graphics_spots = graphics_spots;


	// Create a socket for the client
	// If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		fprintf(stderr, "Problem in creating the socket");
		return EXIT_FAILURE;
	}
	
	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(current_params->IP_address);
	servaddr.sin_port = htons(current_params->port); //convert to big-endian order

	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
	  fprintf(stderr, "Problem in connecting to the server");
	  return EXIT_FAILURE;
	}

	// Put the sockfd into the avatar's bundle of input
	current_params->sockfd = sockfd;
	
	// Create the thread
	threadret = pthread_create(&thread, &attr, avatar, (void*)current_params);
	
	// If there was an error starting up the thread, exit
	if (threadret == 1) {
		fprintf(stderr, "pthread_create failed, rc=%d\n", threadret);
		
		// Cleanup.
		FreeAvatarParam(current_params);
		
		return EXIT_FAILURE;
	}
	
	// Now have one more thread running
	running_threads++;
	
	// Done
	return EXIT_SUCCESS;
}


/* Write a given line to the log file
 *
 * Input:
 * (1) file_name = the name of the log file
 * (2) line = the line to be written to the log
 *
 * Output:
 * (1) exit status
 */
int write_a_line_to_file(char* file_name, char* line){
	// Lock up this function so that we don't have multiple avatars trying to
	// write to the log at the same time
	pthread_mutex_lock(&log_mutex);
	
	// Open up the log file
	FILE *fp = fopen(file_name, "a+");
	
	// Error with file?
	if (fp == NULL){
		return EXIT_FAILURE;
	}
	
	// Write the line to the log
	fprintf(fp, "%s", line);
	
	// Close the file
	fclose(fp);
	
	// Unlock
	pthread_mutex_unlock(&log_mutex);
	
	// Done!
	return EXIT_SUCCESS;
}


/* Function to store all initial information of avatar in a string.
 *
 * Input:
 * (1) params = the bundle of parameters passed to the avatar
 *
 * Output:
 * (1) a summary of these parameters in a string
 */
char* get_line_information(AvatarParameter* params) {

	char *line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	// Will hold main avatar info and be printed to the file
	char* avatar_header = (char*)calloc(MAX_HEADER_LENGTH, sizeof(char));
	
	// Print out variables to make sure everything is assigned
	sprintf(line, "=========================================\n");
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "AVATAR #%d\n", params->ID);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "difficulty: %d\n", params->difficulty);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "IP_address: %s\n", params->IP_address);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));

	sprintf(line, "port: %d\n", (int)params->port);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "log_file: %s\n", params->log_file);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "sockfd: %d\n", params->sockfd);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "total number of avatars: %d\n", params->num_avatars);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "maze width: %d\n", params->width);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "maze height: %d\n", params->height);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "my pointer to the_maze: %p\n", (void*)params->maze);
	strcat(avatar_header, line);
	free(line); line = NULL; line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "=========================================\n\n");
	strcat(avatar_header, line);
	free(line); line = NULL;
	
	avatar_header = realloc(avatar_header, strlen(avatar_header) + 1);
	return avatar_header;
}


/* This function takes a list of everybody's current location in the maze,
 * and one particular avatar's location in the maze, and updates that avatar's\n
 * information.
 *
 * Input:
 * (1) spots = a list of all the avatar's spots in the maze
 * (2) spot = one particular avatar's spot
 * (3) ID = one particular avatar's ID
 *
 * Output: None.
 */
void upload_graphics_info(XYPos** spots, XYPos* spot, int ID){
	// Lock so no collisions
	pthread_mutex_lock(&graphics_mutex);

	// Update spot
	spots[ID] = spot;
	
	pthread_mutex_unlock(&graphics_mutex);
}


/* Draws a frame of the maze graphics
 *
 * Input:
 * (1) spots = a list of every avatar's position in the maze
 * (2) width = the width of the maze
 * (3) height = the height of the maze
 * (4) num_avatars = the number of avatars in the maze
 * (5) turn_count = the number of turns taken so far
 *
 * Output: No return value; prints a frame of the graphics to the console
 */
void maze_graphics(XYPos** spots, int width, int height, int num_avatars, int turn_count){
	int k;
	// If any of the locations of the avatars is NULL, just stop
	for (k = 0; k < num_avatars; k++){
		if (spots[k] == NULL){
			return;
		}
	}
	
	// This arcane looking print statement resets the screen, so that the
	// animation looks like a flip-book
	printf("\033[2J\033[1;1H");
	
	// Avatar at spot will be used to keep track of which and how many avatars
	// are at a spot in the maze
	int avatar_at_spot;
	
	// For every spot in the maze, plus two in each dimension...
	for (int i = -1; i < width + 1; i++){
		for (int j = -1; j < height + 1; j++){
			// -1 means there is nobody at this spot
			avatar_at_spot = -1;
			// If this is an edge spot...
			if (i == -1 || i == width || j == -1 || j == height){
				// Print an edge
				printf("+");
				
				// Print status messages
				if (i == width && j == height){
					printf("\tturns taken: %d", turn_count);
				}
				if (i == width - 1 && j == height){
					if (!PathFound){
						printf("\tStill searching for a path with RHR...");
					}
					else{
						printf("\tPath found!");
					}
				}
			}
			
			// If it's not an edge...
			else{
				// For each avatar...
				for (k = 0; k < num_avatars; k++){
					// If that avatar is at this spot AND nobody else has been
					// seen here yet, avatar_at_spot is this avatar's ID
					if (avatar_at_spot == -1 && (int)spots[k]->x == i && (int)spots[k]->y == j){
						avatar_at_spot = k;
					}
					// If that avatar is at this spot AND somebody else HAS been
					// seen here already, avatar_at_spot is -2, which means multiple
					// avatars
					else if (avatar_at_spot != -1 && (int)spots[k]->x == i && (int)spots[k]->y == j){
						avatar_at_spot = -2;
					}
				}
				// If multiple avatars, print 'X'
				if (avatar_at_spot == -2){
					printf("X");
				}
				// If no avatars, print a blank space
				else if (avatar_at_spot == -1){
					printf(" ");
				}
				// Otherwise there is exactly one avatar there; print its ID
				else{
					printf("%d", avatar_at_spot);
				}
			}
		}
		// Go onto the next line
		printf("\n");
	}
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

	// If the avatar did not move...
	if (CheckSameXYPos(prevPosition,currentPosition)==1)
	{
		// Update that BlockNode's direction as a wall
		
		// Tried to go West?
		if (myDirection==0)
		{
			// Westward is a wall, the spot west of this has an Eastward wall
			Maze[prev_x][prev_y]->West=0;
			if (prev_x > 0){
				Maze[prev_x - 1][prev_y]->East=0;
			}
		}
		
		// Tried to go North?
		if (myDirection==1)
		{
			// Northward is a wall, the spot north of this has an Southern wall
			Maze[prev_x][prev_y]->North=0;
			if (prev_y > 0){
				Maze[prev_x][prev_y - 1]->South=0;
			}
		}
		
		// Tried to go South?
		if (myDirection==2)
		{
			// Southward is a wall, the spot south of this has an northward wall
			Maze[prev_x][prev_y]->South=0;
			if (prev_y < MazeHeight - 1){
				Maze[prev_x][prev_y + 1]->North=0;
			}
		}
		
		// Tried to go East?
		if (myDirection==3)
		{
			// Eastward is a wall, the spot East of this has an Westward wall
			Maze[prev_x][prev_y]->East=0;
			if (prev_x < MazeWidth - 1){
				Maze[prev_x + 1][prev_y]->West=0;
			}
		}
	}
	// Otherwise we were NOT blocked
	else
	{
		//Need to set previous position's as having a possible move, 
		//and then update new locations'BlockNode AvatarVisited Array 
		if ((prev_x==curr_x) && (prev_y>curr_y))
		{
			// Moved North successfully
			Maze[prev_x][prev_y]->North=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->South=1;
			
		}
		else if ((prev_x<curr_x) && (prev_y==curr_y))
		{
			// Moved East successfully
			Maze[prev_x][prev_y]->East=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->West=1;
		}
		else if ((prev_x==curr_x) && (prev_y<curr_y))
		{
			// Moved South successfully
			Maze[prev_x][prev_y]->South=1;
			Maze[curr_x][curr_y]->AvatarVisited[myAvatar]=1;
			Maze[curr_x][curr_y]->North=1;
		}
		else if ((prev_x>curr_x) && (prev_y==curr_y))
		{
			// Moved West successfully
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


/* Free the avatar parameter struct
 *
 * Input:
 * (1) params = the struct to be freed
 *
 * Output: None.
 */
void FreeAvatarParam(AvatarParameter *params) {
	free(params->IP_address);
	free(params->log_file);
	free(params);
	params = NULL;
}