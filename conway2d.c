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
	curr[3][3] = 1;
	curr[4][4] = 1;
	curr[4][5] = 1;
	curr[3][5] = 1;
	curr[2][5] = 1;
}

void spawnGosperGliderGun() { // spawns a gosper glider gun
	curr[6][3] = 1;
	curr[6][4] = 1;
	curr[7][3] = 1;
	curr[7][4] = 1;
	curr[7][11] = 1;
	curr[7][13] = 1;
	curr[8][11] = 1;
	curr[6][13] = 1;
	curr[8][12] = 1;
	curr[6][12] = 1;
	curr[5][25] = 1;
	curr[5][27] = 1;
	curr[6][25] = 1;
	curr[4][27] = 1;
	curr[6][26] = 1;
	curr[4][26] = 1;
	curr[8][19] = 1;
	curr[8][20] = 1;
	curr[9][19] = 1;
	curr[10][19] = 1;
	curr[9][21] = 1;
	curr[11][38] = 1;
	curr[11][39] = 1;
	curr[12][38] = 1;
	curr[13][38] = 1;
	curr[12][40] = 1;
	curr[11][38] = 1;
	curr[16][29] = 1;
	curr[16][28] = 1;
	curr[16][27] = 1;
	curr[17][27] = 1;
	curr[18][28] = 1;
	curr[4][37] = 1;
	curr[4][38] = 1;
	curr[5][37] = 1;
	curr[5][38] = 1;
}

void printBoard(char** board) { // debug prints the board
	int i, j, r;
	MPI_Barrier(MPI_COMM_WORLD);
	for (r = 0; r < mpi_size; r++) {
		if (mpi_rank == r) {
			for (i = 0; r == 0 && i < 50; i++) printf("\n");
			if (mpi_size != 1) printf("Rank %d\n", mpi_rank);
			// if (mpi_size == 1) {
				for (j = 0; j < WIDTH; ++j) printf("%c", (upGhost[j]) ? 'X' : '.'); // prints up ghost row
				printf("\n\n");
			// }
			for (i = 0; i < rows_per_rank; i++) {
				for (j = 0; j < WIDTH; j++) printf("%c", (board[i][j]) ? 'X' : '.');
				printf("\n");
			}
			// if (mpi_size == 1) {
				printf("\n");
				for (j = 0; j < WIDTH; ++j) printf("%c", (downGhost[j]) ? 'X' : '.'); // prints down ghost row
				printf("\n");
			// }
			if (mpi_size != 1) printf("\n");
		}
		fflush(stdout);
		MPI_Barrier(MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
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
		if (mpi_rank == 0) spawnGlider();
	#endif

	#ifdef GOSPER
		if (mpi_rank == 0) spawnGosperGliderGun();
	#endif

	#if !defined(GLIDER) && !defined(GOSPER)
		for (i = 0; i < rows_per_rank; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = 1;
	#endif

	#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
		printBoard(curr);
	#endif

	// Running simulation

	for (t = 0; t < TIME; t++) { // simulation running for TIME ticks
		for (i = 0; i < rows_per_rank; i++) {
			for (j = 0; j < WIDTH; j++) {
				int neighbors = getNeighbors(curr, i, j); 

				#if !defined(GLIDER) && !defined(GOSPER) // RNG for non-preset conway game
					int index = j + ((WIDTH / mpi_size) * mpi_rank); // index = local_row + (rows_per_rank * mpi_rank)
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

		for (i = 0; i < rows_per_rank; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = next[i][j]; // updates board with new data

		if (mpi_size == 1) { // Copy the first and last rows of this rank into the ghosts
			memcpy(upGhost, curr[rows_per_rank-1], WIDTH);
			memcpy(downGhost, curr[0], WIDTH);
		} else {
			MPI_Irecv(upGhost, WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 1, MPI_COMM_WORLD, &request);
			MPI_Isend(curr[rows_per_rank-1], WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 1, MPI_COMM_WORLD, &request3);
			MPI_Irecv(downGhost, WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 2, MPI_COMM_WORLD, &request2);
			MPI_Isend(curr[0], WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 2, MPI_COMM_WORLD, &request4);
			MPI_Wait(&request, &status);
			MPI_Wait(&request2, &status);
			MPI_Wait(&request3, &status);
			MPI_Wait(&request4, &status);
		}


		usleep(50*1000); // sleeps for 50 milliseconds
	
		#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
			MPI_Barrier(MPI_COMM_WORLD);
			printBoard(curr);
		#endif
	}

	// Frees variables

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