#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <mpi.h>

#include "clcg4.h"

#define WIDTH 50
#define HEIGHT 25
#define TIME 300
#define THRESHOLD 0.25

// Global Definitions
int mpi_size = -1, mpi_rank = -1;

void spawnGlider(char* curr) { // spawns a glider to the board
	curr[3 * WIDTH + 3] = 1;
	curr[4 * WIDTH + 4] = 1;
	curr[4 * WIDTH + 5] = 1;
	curr[3 * WIDTH + 5] = 1;
	curr[2 * WIDTH + 5] = 1;
}

void spawnGosperGliderGun(char* curr) { // spawns a gosper glider gun
	curr[6 * WIDTH + 3] = 1;
	curr[6 * WIDTH + 4] = 1;
	curr[7 * WIDTH + 3] = 1;
	curr[7 * WIDTH + 4] = 1;
	curr[7 * WIDTH + 11] = 1;
	curr[7 * WIDTH + 13] = 1;
	curr[8 * WIDTH + 11] = 1;
	curr[6 * WIDTH + 13] = 1;
	curr[8 * WIDTH + 12] = 1;
	curr[6 * WIDTH + 12] = 1;
	curr[5 * WIDTH + 25] = 1;
	curr[5 * WIDTH + 27] = 1;
	curr[6 * WIDTH + 25] = 1;
	curr[4 * WIDTH + 27] = 1;
	curr[6 * WIDTH + 26] = 1;
	curr[4 * WIDTH + 26] = 1;
	curr[8 * WIDTH + 19] = 1;
	curr[8 * WIDTH + 20] = 1;
	curr[9 * WIDTH + 19] = 1;
	curr[10 * WIDTH + 19] = 1;
	curr[9 * WIDTH + 21] = 1;
	curr[11 * WIDTH + 38] = 1;
	curr[11 * WIDTH + 39] = 1;
	curr[12 * WIDTH + 38] = 1;
	curr[13 * WIDTH + 38] = 1;
	curr[12 * WIDTH + 40] = 1;
	curr[11 * WIDTH + 38] = 1;
	curr[16 * WIDTH + 29] = 1;
	curr[16 * WIDTH + 28] = 1;
	curr[16 * WIDTH + 27] = 1;
	curr[17 * WIDTH + 27] = 1;
	curr[18 * WIDTH + 28] = 1;
	curr[4 * WIDTH + 37] = 1;
	curr[4 * WIDTH + 38] = 1;
	curr[5 * WIDTH + 37] = 1;
	curr[5 * WIDTH + 38] = 1;
}

char getBoardElement(char* board, char* upGhost, char* downGhost, int i, int j) { // returns the element stored at board board[i * WIDTH + j]
	if (i < -1) assert(-1);
	if (i > HEIGHT+1) assert(-1);

	if (j < 0) j = j + WIDTH;
	if (j >= WIDTH) j = j % WIDTH;

	if (i == -1) { // access up ghost row
		return (upGhost[j]);
	} else if (i == HEIGHT) { //access down ghost row
		return (downGhost[j]);
	} else { // access main board
		return (board[i*WIDTH + j]);
	}
}

void setBoardElement(char* board, char* upGhost, char* downGhost, int i, int j, char value){
	if (i < -1) assert(-1);
	if (i > HEIGHT+1) assert(-1);

	if (j < 0) j = j+WIDTH;
	if (j >= WIDTH) j = j%WIDTH;

	if (i == -1){ //access ghost row
		upGhost[j] = value;
	} else if (i == HEIGHT){ //access second ghost row
		downGhost[j] = value;
	} else { //Access main board
		board[i*WIDTH + j] = value;
	}
}

void printBoard(char* board, char* upGhost, char* downGhost) { // debug prints the board
	int i, j;
	for (i = 0; i < 50; i++) printf("\n");
	for (j = 0; j < WIDTH; ++j) printf("%c", (upGhost[j]) ? 'X' : '.'); // prints up ghost row
	printf("\n\n");
	for (i = 0; i < HEIGHT; i++) { // prints board
		for (j = 0; j < WIDTH; j++) printf("%c", (board[i * WIDTH + j]) ? 'X' : '.');
		printf("\n");
	}
	printf("\n");
	for (j = 0; j < WIDTH; ++j) printf("%c", (downGhost[j]) ? 'X' : '.'); // prints down ghost row
	printf("\n");
}

int getNeighbors(char* board, char* upGhost, char* downGhost, int i, int j) { // returns the amount of living cells around board[i*WIDTH + j]
	int count = 0;
	if (getBoardElement(board, upGhost, downGhost, i-1, j-1)) count++; // checks if cell is alive at top left most position
	if (getBoardElement(board, upGhost, downGhost, i-1, j  )) count++; // checks if cell is alive at top mid most position
	if (getBoardElement(board, upGhost, downGhost, i-1, j+1)) count++; // checks if cell is alive at top right most position
	if (getBoardElement(board, upGhost, downGhost, i  , j-1)) count++; // checks if cell is alive at mid left most position
	if (getBoardElement(board, upGhost, downGhost, i  , j+1)) count++; // checks if cell is alive at mid right most position
	if (getBoardElement(board, upGhost, downGhost, i+1, j-1)) count++; // checks if cell is alive at bottom left most position
	if (getBoardElement(board, upGhost, downGhost, i+1, j  )) count++; // checks if cell is alive at bottom mid most position
	if (getBoardElement(board, upGhost, downGhost, i+1, j+1)) count++; // checks if cell is alive at bottom right most position
	return count;
}

int main(int argc, char *argv[]) {
	// Initializes variables, RNG, and MPI

	int i, j, t;
	char *curr, *next, *upGhost, *downGhost;
	curr = (char*)calloc(HEIGHT * WIDTH, sizeof(char));
	next = (char*)calloc(HEIGHT * WIDTH, sizeof(char));

	InitDefault();

	MPI_Init( &argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  upGhost = (char*)calloc(WIDTH, sizeof(char));
	downGhost = (char*)calloc(WIDTH, sizeof(char));


	// Sets and prints the intial state of the board

	#ifdef GLIDER
		spawnGlider(curr);
	#endif

	#ifdef GOSPER
		spawnGosperGliderGun(curr);
	#endif

	#if !defined(GLIDER) && !defined(GOSPER)
		for (i = 0; i < HEIGHT; i++) for (j = 0; j < WIDTH; j++) curr[i * WIDTH + j] = 1;
	#endif

	#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
		printBoard(curr, upGhost, downGhost);
	#endif

	// Running simulation

	for (t = 0; t < TIME; t++) { // simulation running for TIME ticks
		for (i = 0; i < HEIGHT; i++) {
			for (j = 0; j < WIDTH; j++) {
				int neighbors = getNeighbors(curr, upGhost, downGhost, i, j); // index = j + (WIDTH * 0); // index = local_row + (rows_per_rank * mympirank)
				
				#if !defined(GLIDER) && !defined(GOSPER) // RNG for non-preset conway game
					int index = j + ((WIDTH / mpi_size) * mpi_rank);
					double value = GenVal(index);
					if (THRESHOLD >= value) {
						next[i * WIDTH + j] = (value < 0.5 * THRESHOLD) ? 0 : 1;
						continue;
					}
				#endif

				if (curr[i * WIDTH + j]) next[i * WIDTH + j] = (neighbors == 2 || neighbors == 3) ? 1 : 0; // if living cell has 2-3 neighbors, it lives, else dies
				else next[i * WIDTH + j] = (neighbors == 3) ? 1 : 0; // if dead cell has 3 neighbors, new one is born, else still dead
			}
		}

		for (i = 0; i < HEIGHT; i++) for (j = 0; j < WIDTH; j++) curr[i * WIDTH + j] = next[i * WIDTH + j]; // updates board with new data

		if (mpi_size == 1) { // Copy the first and last rows of this rank into the ghosts
			memcpy(upGhost, &(curr[(HEIGHT-1)*WIDTH]), WIDTH);
			memcpy(downGhost, curr, WIDTH);
		} else {
			
		}

		usleep(50*1000); // sleeps for 50 milliseconds
	
		#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
			printBoard(curr, upGhost, downGhost);
		#endif
	}

	// Frees variables and exits MPI

	free(curr);
	free(next);
	free(upGhost);
	free(downGhost);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();	
	return 0;
}
