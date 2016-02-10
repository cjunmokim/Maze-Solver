/* ========================================================================== */
/* File: AMStartup.c - Maze Runner
 *
 * Authors: Nick Schwartz, Will McConnell, Caleb Junmo Kim
 * Date: August 20, 2015
 *
 * Description:  This file creates an initial connection with the server and starts up the
 * 				 avatar threads.
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
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
#include <unistd.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"
#include "maze.h"
#include "file.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables
int running_threads = 0;

// ---------------- Private prototypes
int Check_If_A_File_Is_Empty(char *);

/* ========================================================================== */


int main (int argc, char *argv[]) {

	// Declare variables.
	int sockfd;
    struct sockaddr_in servaddr;
	uint32_t nAvatar;
	uint32_t difficulty;
	char *ip_address;
	
	// Check arguments.
	
	// Check that there are three input parameters passed.
	if (argc != 4) {
		printf("Please input three parameters exactly.\n");
		printf("Usage: ./AMStartup [nAvatars] [Difficulty] [Hostname]\n");
		return EXIT_FAILURE;
	}
	
	// Check that the number of Avatars passed is a number.
	// Loop through each letter of the first argument and check that it is indeed a number.
	for (int i = 0; i < strlen(argv[1]); i++) {
		if (!isdigit((int)argv[1][i])) {
 			printf("Please input a valid number for the number of Avatars.\n");
   			return EXIT_FAILURE;
   		}
	}
   	
  	sscanf(argv[1], "%u", &nAvatar); // Store the argument as an integer.
	
	// Num avatar check
	if (nAvatar < 2 || nAvatar > 10){
		printf("Must have between two and ten avatars.\n");
		return EXIT_FAILURE;
	}
	
	// Check that the difficulty passed is a number.
	// Loop through each letter of the second argument and check that it is indeed a number.
	for (int i = 0; i < strlen(argv[2]); i++) {
		if (!isdigit((int)argv[2][i])) {
 			printf("Please input a valid number for the difficulty level.\n");
   			return EXIT_FAILURE;
   		}
	}
   	
  	sscanf(argv[2], "%u", &difficulty); // Store the argument as an integer.
	
	// Difficulty check
	if (difficulty < 0 || difficulty > 9){
		printf("Difficulty must be between 0 and 9.\n");
		return EXIT_FAILURE;
	}
	
	// Get IP address
	struct hostent* host_info = gethostbyname(argv[3]);
	if (host_info == NULL){
		printf("ERROR: Bad host name\n");
		return EXIT_FAILURE;
	}
	
	// Assign IP address to ip_address
	struct in_addr** addresses;
	addresses = (struct in_addr **)host_info->h_addr_list;
	ip_address = inet_ntoa(*addresses[0]);
	
	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		fprintf(stderr, "Problem in creating the socket");
		return EXIT_FAILURE;
	}
	
	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(ip_address);
	servaddr.sin_port = htons(AM_SERVER_PORT); //convert to big-endian order

	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		fprintf(stderr, "Problem in connecting to the server");
		return EXIT_FAILURE;
	}
	
	// Create the AM_INIT message to send to the server.
	AM_Message init_msg;
	memset(&init_msg, 0, sizeof(AM_Message));
	init_msg.type = htonl(AM_INIT);
	init_msg.init.nAvatars = htonl(nAvatar);
	init_msg.init.Difficulty = htonl(difficulty);
	
	// Send the AM_INIT message.
	send(sockfd, &init_msg, sizeof(init_msg), 0);
	
	AM_Message rec_msg;
	
	// Receive the AM_INIT_OK message.
	if (recv(sockfd, &rec_msg, sizeof(rec_msg), 0) < 0) {
		fprintf(stderr, "Error receiving the AM_INIT_OK message\n");
		return EXIT_FAILURE;
	}
	
	// Check that the return message is of the correct type: AM_INIT_OK.
	if (ntohl(rec_msg.type) != AM_INIT_OK) {
		fprintf(stderr, "Error receiving the AM_INIT_OK message; type is not AM_INIT_OK\n");
		return EXIT_FAILURE;
	}
	
	// Store the AM_INIT_OK parameters into local variables.
	uint32_t port = ntohl(rec_msg.init_ok.MazePort);
	uint32_t width = ntohl(rec_msg.init_ok.MazeWidth);
	uint32_t height = ntohl(rec_msg.init_ok.MazeHeight);	
	
	// Check if ./results exists as a directory
	if (IsDir("./results") == 0){
		fprintf(stderr, "ERROR: must have a directory in project named \"results\"");
		return EXIT_FAILURE;
	}
	
	// Create the maze
	BlockNode*** maze = InitializeArray(width, height, nAvatar);
	
	// Create a log file.
	char *user;
	user = getlogin(); // Get the username.
	char *filename = (char *)calloc(1, MAX_LINE_LENGTH);
	
	// line will be used to print out messages to the log
	char *line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	// Create file name
	sprintf(filename, "./results/Amazing_%s_%u_%u.log", user, nAvatar, difficulty);
	filename = realloc(filename, strlen(filename) + 1);
	
	// If there is already a file in this directory with that name, delete it
	char* del_command = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
	sprintf(del_command, "rm %s > /dev/null 2>&1", filename);
	system(del_command);
	
	free(del_command);
	del_command = NULL;
	
	FILE *fp = fopen(filename, "a+"); // Open up a file to write the log to.
	
	// Get the time.
	time_t current_time;
	time(&current_time);
	char *time = ctime(&current_time);
	
	fprintf(fp, "%s, %u, %s\n", user, port, time); // Write the first line of the log.
	
	// Print info about the maze
	sprintf(line, "*****************\n Maze Width: %u\n Maze Height: %u\n*****************\n\n", width, height);
	fprintf(fp, "%s", line);
	free(line); line = NULL; 
	line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
	
	sprintf(line, "The maze structure lives at: %p\n\n", (void*)maze);
	fprintf(fp, "%s", line);
	free(line); line = NULL;
	
	fclose(fp);
	
	// Initialize variable for graphics.
	XYPos** graphics_spots = (XYPos**)calloc(nAvatar, sizeof(XYPos*));
	
	
	// Run all the avatars.
	for (int i = 0; i < nAvatar; i++) {
		if (avatar_run(i, difficulty, ip_address, port, filename, nAvatar, (int)width, (int)height, maze, graphics_spots) == EXIT_FAILURE){
			fprintf(stderr, "Error creating threads\n");
			return EXIT_FAILURE;
		}
	}
	
	// Don't terminate until the threads are all done.
	while (running_threads > 0) {
		sleep(1);
	}
	
	// Cleanup.
	free(filename);
	freeMaze(maze, (int)width, (int)height); // Free the maze.
	free(graphics_spots);
	
	pthread_exit(NULL);
	// return EXIT_SUCCESS;
}

