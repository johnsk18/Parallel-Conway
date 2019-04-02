// Team Members:
//  Kaylan Johnson (johnsk18) - johnsk18@rpi.edu
//  Zac Silverman (silvez) - silvez@rpi.edu
//  River Steed (steede2) - steede2@rpi.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "clcg4.h"
#include <mpi.h>
#include <pthread.h>

// #define BGQ 1 // when running BG/Q, comment out when testing on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#define processor_frequency 1600000000.0 // processing speed for BG/Q
#else
#define GetTimeBase MPI_Wtime
#define processor_frequency 1.0            
#endif

#define WIDTH (1 << 6) // 1 << x is the same as 2 ^ x
#define HEIGHT (1 << 5) // mpi_size * NUMTHREADS <= HEIGHT
#define TIME 1
#define THRESHOLD 0.25
#define NUMTHREADS 4 // including main thread

// Global Definitions
char **curr, **next, *upGhost, *downGhost;
double g_time_in_secs = 0, g_start_cycles = 0, g_end_cycles = 0;
long long total_alive_cells = 0, alive_cells_rank = 0, *alive_cells_all;
int mpi_size = -1, mpi_rank = -1, rows_per_rank = -1, rows_per_thread = -1;
MPI_Request request, request2, request3, request4;
MPI_Status status;
pthread_barrier_t barrier; 
pthread_t* threadIDs = NULL;
pthread_mutex_t mutexalive = PTHREAD_MUTEX_INITIALIZER;

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
		fflush(stdout);
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

void* updateRows(void* arg) { // thread function used to update rows
	int i, j, t = (int) (long) arg; // t stands as a simple thread id to determine its rows to update

	// Updates board based on t (the thread number) using game logic/randomness and ghost rows

	for (i = 0 + (t * rows_per_thread); i < rows_per_thread + (t * rows_per_thread); i++) { // row
		for (j = 0; j < WIDTH; j++) { // column
			int neighbors = getNeighbors(curr, i, j); 

			#if !defined(GLIDER) && !defined(GOSPER) // RNG for non-preset conway game
				int index = i + (rows_per_rank * mpi_rank);
				double value = GenVal(index); // each row has its own RNG stream
				if (THRESHOLD > value) { // if below threshold, randomize update
					next[i][j] = (value < 0.5 * THRESHOLD) ? 0 : 1;
					continue;
				}
			#endif

			if (curr[i][j]) next[i][j] = (neighbors == 2 || neighbors == 3) ? 1 : 0; // if living cell has 2-3 neighbors, it lives, else dies
			else next[i][j] = (neighbors == 3) ? 1 : 0; // if dead cell has 3 neighbors, new one is born, else still dead
		}
	}
	
	pthread_mutex_lock (&mutexalive);
	for (i = 0 + (t * rows_per_thread); i < rows_per_thread + (t * rows_per_thread); i++) for (j = 0; j < WIDTH; j++) alive_cells_rank += next[i][j];
	pthread_mutex_unlock (&mutexalive);
	
	if (t != 0) pthread_exit(NULL); // if not main thread, exit
	return NULL;
}

int main(int argc, char *argv[]) {
	// Initializes variables, thread barrier, and MPI

	int i, j, k, t;
	InitDefault();
	MPI_Init( &argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	if (mpi_rank == 0) g_start_cycles = GetTimeBase();
	rows_per_rank = HEIGHT / mpi_size;
	rows_per_thread = rows_per_rank / NUMTHREADS;
	alive_cells_all = (mpi_rank == 0) ? (long long*)calloc(TIME, sizeof(long long)) : NULL;
	if (NUMTHREADS > 1) threadIDs = calloc(NUMTHREADS - 1, sizeof(pthread_t));
	pthread_barrier_init(&barrier, NULL, NUMTHREADS);

	curr = (char**)calloc(rows_per_rank, sizeof(char*));
	next = (char**)calloc(rows_per_rank, sizeof(char*));
	for (i = 0; i < rows_per_rank; i++) curr[i] = (char*)calloc(WIDTH, sizeof(char));
	for (i = 0; i < rows_per_rank; i++) next[i] = (char*)calloc(WIDTH, sizeof(char));
 	upGhost = (char*)calloc(WIDTH, sizeof(char));
	downGhost = (char*)calloc(WIDTH, sizeof(char));

	#ifdef DEBUG
		if (mpi_rank == 0) printf("Ranks: %d\nThreads: %d\nBoard: %d x %d\nTicks: %d\n", mpi_size, NUMTHREADS, HEIGHT, WIDTH, TIME);
	#endif

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

	MPI_Barrier( MPI_COMM_WORLD );

	// Simulation running for TIME ticks

	for (t = 0, k = 0; t <= TIME; t++, k = 0) { 
		// Updates ghost rows with data from other ranks

		MPI_Irecv(upGhost, WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 1, MPI_COMM_WORLD, &request);
		MPI_Isend(curr[rows_per_rank-1], WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 1, MPI_COMM_WORLD, &request3);
		MPI_Irecv(downGhost, WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 2, MPI_COMM_WORLD, &request2);
		MPI_Isend(curr[0], WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 2, MPI_COMM_WORLD, &request4);
		MPI_Wait(&request, &status);
		MPI_Wait(&request2, &status);
		MPI_Wait(&request3, &status);
		MPI_Wait(&request4, &status);
	
		#ifdef BOARD // sleeps and prints board
			usleep(50*1000); // sleeps for 50 milliseconds for debugging purposes
			printBoard(curr);
		#endif

		if (t == TIME) break; // ends loop when time has been hit after final ghost rows and board has printed

		// Updates rows via threads (including main thread)

		updateRows((void *) (long) k);
		for (k = 1; k < NUMTHREADS; k++) pthread_create(&threadIDs[k-1], NULL, updateRows, (void *) (long) k); // creates threads
		for (k = 0; k < NUMTHREADS - 1; k++) pthread_join(threadIDs[k], NULL); // joins threads

		for (i = 0; i < rows_per_rank; i++) for (j = 0; j < WIDTH; j++) curr[i][j] = next[i][j]; // updates current board with new data
		
   
		MPI_Reduce(&alive_cells_rank, &alive_cells_all[t], 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	if (mpi_rank == 0) {
    g_end_cycles = GetTimeBase();
    g_time_in_secs = ((double)(g_end_cycles - g_start_cycles)) / processor_frequency;
		for (int k = 0; k < TIME; k++) {
			printf("Live cells from tick %d: %lld\n", k, alive_cells_all[k]);
			total_alive_cells += alive_cells_all[k];
		}
		printf("Finished simulation in %f seconds. Number of alive cells: %lld\n", g_time_in_secs, total_alive_cells);
		#ifdef BOARD
			printf("Ranks: %d\nThreads: %d\nBoard: %d x %d\nTicks: %d\n", mpi_size, NUMTHREADS, HEIGHT, WIDTH, TIME);
		#endif	
	}

	// Frees variables and finalizes MPI

	for (i = 0; i < rows_per_rank; i++) free(curr[i]);
	for (i = 0; i < rows_per_rank; i++) free(next[i]);
	free(curr);
	free(next);
	free(upGhost);
	free(downGhost);
	if (threadIDs) free(threadIDs);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}