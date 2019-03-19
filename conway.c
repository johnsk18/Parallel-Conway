#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 50
#define HEIGHT 25
#define TIME 300

int **curr, **next;

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

void printBoard(int** board) { // prints the board
	int i, j;
	for (i = 0; i < 50; i++) printf("\n");
	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) printf("%c", (board[i][j]) ? 'X' : '.');
		printf("\n");
	}
}

int getNeighbors(int** board, int i, int j) { // returns the amount of living cells around board[i][j]
	int count = 0, i_, j_;

	if (board[((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at top left most position
	if (board[((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j-0)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at top mid most position
	if (board[((i_ = (i-1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at top right most position
	if (board[((i_ = (i-0)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at mid left most position
	if (board[((i_ = (i-0)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at mid right most position
	if (board[((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j-1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at bottom left most position
	if (board[((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j-0)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at bottom mid most position
	if (board[((i_ = (i+1)%HEIGHT) < 0) ? HEIGHT + i_ : i_][((j_ = (j+1)%WIDTH) < 0) ? WIDTH + j_ : j_]) count++; // checks if cell is alive at bottom right most position

	return count;
}

int main(int argc, char *argv[]) {
	int i,j,t;
	curr = (int**)calloc(HEIGHT, sizeof(int*));
	next = (int**)calloc(HEIGHT, sizeof(int*));
	for (i = 0; i < HEIGHT; i++) curr[i] = (int*)calloc(WIDTH, sizeof(int));
	for (i = 0; i < HEIGHT; i++) next[i] = (int*)calloc(WIDTH, sizeof(int));

	// spawnGlider();
	spawnGosperGliderGun();

	printBoard(curr);

	for (t = 0; t < TIME; t++) { // simulation running for TIME ticks
		for (i = 0; i < HEIGHT; i++) {
			for (j = 0; j < WIDTH; j++) {
				int neighbors = getNeighbors(curr, i, j);
				if (curr[i][j]) next[i][j] = (neighbors == 2 || neighbors == 3) ? 1 : 0; // under/over/suitable population
				else if (neighbors == 3) next[i][j] = 1; // reproduction
				else next[i][j] = 0; // death
			}
		}

		for (i = 0; i < HEIGHT; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = next[i][j]; // update board

		usleep(100*1000);
		printBoard(curr);
	}

	for (i = 0; i < HEIGHT; i++) free(curr[i]);
	for (i = 0; i < HEIGHT; i++) free(next[i]);
	free(curr);
	free(next);

	return 0;
}