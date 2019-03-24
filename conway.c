#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "clcg4.h"

#define WIDTH 50
#define HEIGHT 25
#define TIME 100
#define THRESHOLD 0.25

// Global Definitions
int *curr, *next;

void spawnGlider() { // spawns a glider to the board
	curr[3 * WIDTH + 3] = 1;
	curr[4 * WIDTH + 4] = 1;
	curr[4 * WIDTH + 5] = 1;
	curr[3 * WIDTH + 5] = 1;
	curr[2 * WIDTH + 5] = 1;
}

void spawnGosperGliderGun() { // spawns a gosper glider gun
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

void printBoard(int* board) { // debug prints the board
	int i, j;
	for (i = 0; i < 50; i++) printf("\n");
	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) printf("%c", (board[i * WIDTH + j]) ? 'X' : '.');
		printf("\n");
	}
}

int getNeighbors(int* board, int i, int j) { // returns the amount of living cells around board[i * WIDTH + j]
	int count = 0, i_, j_; // i_ and j_ represent the modded coordinates, if negative, they get added to either the baord height or length respectively to simulate wrap-around
	if (board[(((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at top left most position
	if (board[(((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j-0)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at top mid most position
	if (board[(((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at top right most position
	if (board[(((i_ = (i-0)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at mid left most position
	if (board[(((i_ = (i-0)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at mid right most position
	if (board[(((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at bottom left most position
	if (board[(((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j-0)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at bottom mid most position
	if (board[(((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_) * WIDTH + (((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_)]) count++; // checks if cell is alive at bottom right most position
	return count;
}

int main(int argc, char *argv[]) {
	// Initializes variables

	int i, j, t;
	curr = (int*)calloc(HEIGHT * WIDTH, sizeof(int));
	next = (int*)calloc(HEIGHT * WIDTH, sizeof(int));
	// for (i = 0; i < HEIGHT; i++) curr[i] = (int*)calloc(WIDTH, sizeof(int));
	// for (i = 0; i < HEIGHT; i++) next[i] = (int*)calloc(WIDTH, sizeof(int));

	InitDefault();

	// Sets and prints the intial state of the board

	#ifdef GLIDER
		spawnGlider();
	#endif

	#ifdef GOSPER
		spawnGosperGliderGun();
	#endif

	#if !defined(GLIDER) && !defined(GOSPER)
		for (i = 0; i < HEIGHT; i++) for (j = 0; j < WIDTH; j++) curr[i * WIDTH + j] = 1;
	#endif

	#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
		printBoard(curr);
	#endif

	// Running simulation

	for (t = 0; t < TIME; t++) { // simulation running for TIME ticks
		for (i = 0; i < HEIGHT; i++) {
			for (j = 0; j < WIDTH; j++) {
				int neighbors = getNeighbors(curr, i, j); // index = j + (WIDTH * 0); // index = local_row + (rows_per_rank * mympirank)
				// double value = GenVal(index);
				// if (THRESHOLD >= value) next[i * WIDTH + j] = (value < 0.5 * THRESHOLD) ? 0 : 1;
				// else {
					if (curr[i * WIDTH + j]) next[i * WIDTH + j] = (neighbors == 2 || neighbors == 3) ? 1 : 0; // if living cell has 2-3 neighbors, it lives, else dies
					else next[i * WIDTH + j] = (neighbors == 3) ? 1 : 0; // if dead cell has 3 neighbors, new one is born, else still dead
				// }
			}
		}

		// #if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
		// 	printBoard(next);
		// #endif

		for (i = 0; i < HEIGHT; i++) for (j = 0; j < WIDTH; j++) curr[i * WIDTH + j] = next[i * WIDTH + j]; // updates board with new data

		usleep(50*1000); // sleeps for 50 milliseconds
	
		#if defined(GLIDER) || defined(GOSPER) || defined(BOARD)
			printBoard(curr);
		#endif
	}

	// Frees variables

	// for (i = 0; i < HEIGHT; i++) free(curr[i]);
	// for (i = 0; i < HEIGHT; i++) free(next[i]);
	free(curr);
	free(next);

	return 0;
}