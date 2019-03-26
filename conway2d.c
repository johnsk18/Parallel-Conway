#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "clcg4.h"
#include <mpi.h>

#define WIDTH 64
#define HEIGHT 32
#define TIME 250
#define THRESHOLD 0.25

// Global Definitions
char **curr, **next, *upGhost, *downGhost;
int mpi_size = -1, mpi_rank = -1, rows_per_rank = -1;
MPI_Request request, request2, request3, request4;
MPI_Status status;

void spawnGlider() { // spawns a glider to the board
	int n;
	if ((n = 3 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][3] = 1;
	if ((n = 4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][4] = 1;
	if ((n = 4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][5] = 1;
	if ((n = 3 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][5] = 1;
	if ((n = 2 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][5] = 1;
}

void spawnGosperGliderGun() { // spawns a gosper glider gun
	int n;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][3] = 1;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][4] = 1;
	if ((n =  7 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][3] = 1;
	if ((n =  7 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][4] = 1;
	if ((n =  7 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][11] = 1;
	if ((n =  7 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][13] = 1;
	if ((n =  8 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][11] = 1;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][13] = 1;
	if ((n =  8 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][12] = 1;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][12] = 1;
	if ((n =  5 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][25] = 1;
	if ((n =  5 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][27] = 1;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][25] = 1;
	if ((n =  4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][27] = 1;
	if ((n =  6 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][26] = 1;
	if ((n =  4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][26] = 1;
	if ((n =  8 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][19] = 1;
	if ((n =  8 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][20] = 1;
	if ((n =  9 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][19] = 1;
	if ((n = 10 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][19] = 1;
	if ((n =  9 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][21] = 1;
	if ((n = 11 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
	if ((n = 11 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][39] = 1;
	if ((n = 12 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
	if ((n = 13 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
	if ((n = 12 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][40] = 1;
	if ((n = 11 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
	if ((n = 16 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][29] = 1;
	if ((n = 16 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][28] = 1;
	if ((n = 16 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][27] = 1;
	if ((n = 17 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][27] = 1;
	if ((n = 18 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][28] = 1;
	if ((n =  4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][37] = 1;
	if ((n =  4 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
	if ((n =  5 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][37] = 1;
	if ((n =  5 - (mpi_rank * rows_per_rank)) < rows_per_rank && n >= 0) curr[n][38] = 1;
}

void printBoard(char** board) { // debug prints the board
	int i, j, r;
	MPI_Barrier(MPI_COMM_WORLD);
	for (r = 0; r < mpi_size; r++) {
		if (mpi_rank == r) {
			for (i = 0; i < 1 || (r == 0 && i < 50); i++) printf("\n");
			for (j = 0; j < WIDTH; ++j) printf("%c", (upGhost[j]) ? 'X' : '.'); // prints up ghost row
			printf(" Rank %d\n\n", mpi_rank);
			for (i = 0; i < rows_per_rank; i++) {
				for (j = 0; j < WIDTH; j++) printf("%c", (board[i][j]) ? 'X' : '.');
				printf("\n");
			}
			printf("\n");
			for (j = 0; j < WIDTH; ++j) printf("%c", (downGhost[j]) ? 'X' : '.'); // prints down ghost row
			printf("\n");
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
}

int getCell(char** board, int i, int j) {
	if (i < -1 || i > rows_per_rank) assert(-1);

	if (j < 0) j += WIDTH;
	if (j >= WIDTH) j %= WIDTH;

	if (i == -1) return upGhost[j]; // access up ghost row
	else if (i == rows_per_rank) return downGhost[j]; // access down ghost row
	else return board[i][j]; // access board
}

int getNeighbors(char** board, int i, int j) { // returns the amount of living cells around board[i][j]
	int count = 0;
	if (getCell(board, i-1, j-1)) count++; // checks if cell is alive at top left most position
	if (getCell(board, i-1, j  )) count++; // checks if cell is alive at top mid most position
	if (getCell(board, i-1, j+1)) count++; // checks if cell is alive at top right most position
	if (getCell(board, i  , j-1)) count++; // checks if cell is alive at mid left most position
	if (getCell(board, i  , j+1)) count++; // checks if cell is alive at mid right most position
	if (getCell(board, i+1, j-1)) count++; // checks if cell is alive at bottom left most position
	if (getCell(board, i+1, j  )) count++; // checks if cell is alive at bottom mid most position
	if (getCell(board, i+1, j+1)) count++; // checks if cell is alive at bottom right most position
	return count;
}

int main(int argc, char *argv[]) {
	// Initializes variables

	int i, j, t;
	InitDefault();
	MPI_Init( &argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	rows_per_rank = HEIGHT / mpi_size;
	curr = (char**)calloc(rows_per_rank, sizeof(char*));
	next = (char**)calloc(rows_per_rank, sizeof(char*));
	for (i = 0; i < rows_per_rank; i++) curr[i] = (char*)calloc(WIDTH, sizeof(char));
	for (i = 0; i < rows_per_rank; i++) next[i] = (char*)calloc(WIDTH, sizeof(char));

  upGhost = (char*)calloc(WIDTH, sizeof(char));
	downGhost = (char*)calloc(WIDTH, sizeof(char));

	// Sets and prints the intial state of the board

	#ifdef GLIDER
		spawnGlider();
	#endif

	#ifdef GOSPER
		spawnGosperGliderGun();
	#endif

	#if !defined(GLIDER) && !defined(GOSPER) // intializes board to 1 for non-preset compilations
		for (i = 0; i < rows_per_rank; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = 1;
	#endif

	#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
		printBoard(curr);
	#endif

	// Running simulation

	for (t = 0; t < TIME; t++) { // simulation running for TIME ticks
		// Updates ghost rows with data from other ranks

		MPI_Irecv(upGhost, WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 1, MPI_COMM_WORLD, &request);
		MPI_Isend(curr[rows_per_rank-1], WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 1, MPI_COMM_WORLD, &request3);
		MPI_Irecv(downGhost, WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 2, MPI_COMM_WORLD, &request2);
		MPI_Isend(curr[0], WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 2, MPI_COMM_WORLD, &request4);
		MPI_Wait(&request, &status);
		MPI_Wait(&request2, &status);
		MPI_Wait(&request3, &status);
		MPI_Wait(&request4, &status);
	
		#if defined(GLIDER) || defined(GOSPER) || defined(BOARD) // sleeps and prints board
			usleep(50*1000); // sleeps for 50 milliseconds for debugging purposes
			printBoard(curr);
		#endif

		// Updates board using game logic/randomness and ghost rows

		for (i = 0; i < rows_per_rank; i++) { 
			for (j = 0; j < WIDTH; j++) {
				int neighbors = getNeighbors(curr, i, j); 

				#if !defined(GLIDER) && !defined(GOSPER) // RNG for non-preset conway game
					int index = j + (rows_per_rank * mpi_rank); // index = local_row + (rows_per_rank * mpi_rank)
					double value = GenVal(index);
					if (THRESHOLD >= value) {
						next[i][j] = (value < 0.5 * THRESHOLD) ? 0 : 1;
						continue;
					}
				#endif

				if (curr[i][j]) next[i][j] = (neighbors == 2 || neighbors == 3) ? 1 : 0; // if living cell has 2-3 neighbors, it lives, else dies
				else next[i][j] = (neighbors == 3) ? 1 : 0; // if dead cell has 3 neighbors, new one is born, else still dead
			}
		}

		for (i = 0; i < rows_per_rank; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = next[i][j]; // updates current board with new data
	}

	// Frees variables and finalizes MPI

	for (i = 0; i < rows_per_rank; i++) free(curr[i]);
	for (i = 0; i < rows_per_rank; i++) free(next[i]);
	free(curr);
	free(next);
	free(upGhost);
	free(downGhost);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}