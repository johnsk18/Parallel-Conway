// Team Members:
//	Kaylan Johnson (johnsk18) - johnsk18@rpi.edu
//	Zac Silverman (silvez) - silvez@rpi.edu
//	River Steed (steede2) - steede2@rpi.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <mpi.h>
#include "clcg4.h"

//#define BGQ 1 // when running BG/Q, comment out when testing on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#define processor_frequency 1600000000.0 // processing speed for BG/Q
#else
#define GetTimeBase MPI_Wtime
#define processor_frequency 1.0            
#endif

#define WIDTH (1 << 15) // 1 << x is the same as 2 ^ x
#define HEIGHT (1 << 15) // mpi_size * num_threads <= HEIGHT
#define TIME 1
#define THRESHOLD 1

// Global Definitions
char **curr, *upGhost, *downGhost;
double g_time_in_secs = 0, g_start_cycles = 0, g_end_cycles = 0;
long long total_alive_cells = 0, alive_cells_rank = 0, *alive_cells_all;
int mpi_size = -1, mpi_rank = -1, rows_per_rank = -1, rows_per_thread = -1, num_threads = -1;
MPI_Request request, request2, request3, request4;
MPI_Status status;
pthread_barrier_t barrier; 
pthread_t* threadIDs = NULL;
pthread_mutex_t mutexalive = PTHREAD_MUTEX_INITIALIZER;

typedef struct ColorType {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Color;

Color getColor (unsigned short s) { 
//This creates a map from Black to red to yellow to green to blue to white.
  Color c = {0, 0, 0};
  if (s < 256){
    c.r = s%256;
    c.g = 0;
    c.b = 0;
  } else if (s < 512){
    c.r = 255;
    c.g = s-256;
    c.b = 0;
  } else if (s < 768){
    c.r = 255-(s%256);
    c.g = 255;
    c.b = s%256;
  } else if (s < 1024){
    c.r = s%256;
    c.g = 255;
    c.b = 255;
  } else {
    c.r = 255;
    c.g = 255;
    c.b = 255;
  }
  return c;
}

char saveMap(unsigned short ** heatmap) {
  char * filename;
  int width = WIDTH/32, height = HEIGHT/32;
  //Dumb way of determining the filename without needing fancy logic for it.
  if (THRESHOLD == 0)         filename = "map00.ppm";
  else if (THRESHOLD == 0.25) filename = "map25.ppm";
  else if (THRESHOLD == 0.50) filename = "map50.ppm"; 
  else if (THRESHOLD == 0.75) filename = "map75.ppm";
  else if (THRESHOLD == 1)    filename = "map100.ppm";
  else filename = "map??.ppm";

  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr,"Unable to open %s for writing\n", filename);
    return 0;
  }
  // header information
  fprintf (file, "P6\n");
  fprintf (file, "%d %d\n", width, height);
  fprintf (file, "255\n");

  // the data
  // (0,0) is the bottom left corner this way.
  for (int y = height-1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      Color color = getColor(heatmap[x][y]);
      //printf("%d,%d:r=%d,g=%d,b=%d\n", x, y, color.r, color.g, color.b);
      fputc(color.r, file);
      fputc(color.g, file);
      fputc(color.b, file);
    }
  }
  fclose(file);
  return 1;
}

void saveTextMap(unsigned short ** heatmap){
  char * filename;
  int width = WIDTH/32, height = HEIGHT/32;
  //Dumb way of determining the filename without needing fancy logic for it.
  if (THRESHOLD == 0)         filename = "map00.txt";
  else if (THRESHOLD == 0.25) filename = "map25.txt";
  else if (THRESHOLD == 0.50) filename = "map50.txt"; 
  else if (THRESHOLD == 0.75) filename = "map75.txt";
  else if (THRESHOLD == 1)    filename = "map100.txt";
  else filename = "map??.txt";

  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr,"Unable to open %s for writing\n", filename);
    return;
  }

  for (int y = 0; y < height; ++y){
  	for (int x = 0; x < width; ++x){
  		fprintf(file, "%d ", heatmap[x][y]);
  	}
  	fprintf(file,"\n");
  }
}

int getNeighbors(char** board, int i, int j) { // returns the amount of living cells around board[i][j], optimized for minimal computation
	int left = (j == 0) ? WIDTH - 1 : j - 1; // determines whether to wrap around to the right
	int right = (j == WIDTH - 1) ? 0 : j + 1; // determines whether to wrap around to the left
	char* up = (i == 0) ? upGhost : board[i-1]; // determines whether to use top ghost row or not
	char* down = (i == rows_per_rank - 1) ? downGhost : board[i+1]; // determines where to use bottom ghost row or not
	return up[left] + up[j] + up[right] + board[i][left] + board[i][right] + down[left] + down[j] + down[right]; // returns sum of neighbors
}

void* updateRows(void* arg) { // thread function used to update rows
	int i, j, t, thread_num = (int) (long) arg; // thread_num stands as a simple thread id to determine its rows to update

	for (t = 0; t < TIME; ++t) { // Simulation running for TIME ticks
		// Updates board based on thread_num (the thread number) using game logic/randomness and ghost rows	

		for (i = 0 + (thread_num * rows_per_thread); i < rows_per_thread + (thread_num * rows_per_thread); ++i) { // row


			//double value = GenVal(i + (rows_per_rank * mpi_rank)); // each row has its own RNG stream


			for (j = 0; j < WIDTH; ++j) { // column

				double value = GenVal(i + (rows_per_rank * mpi_rank)); // each row has its own RNG stream

				if (THRESHOLD > value) curr[i][j] = (value < 0.5 * THRESHOLD) ? 0 : 1; // if below threshold, randomize update
				else { // if cell (dead or alive) has 3 neighbors OR has 3 neighbors while living, it lives, else it dies
					int neighbors = getNeighbors(curr, i, j);
					curr[i][j] = (neighbors == 3 || (curr[i][j] && neighbors == 2)) ? 1 : 0;  
				}
			}
		}
		
		pthread_barrier_wait(&barrier); // DO NOT MOVE THIS. THIS BARRIER PLACEMENT IS CRUCIAL!

		// Counts all live cells to alive_cells_rank in a critical section

		pthread_mutex_lock (&mutexalive);
		for (i = 0 + (thread_num * rows_per_thread); i < rows_per_thread + (thread_num * rows_per_thread); ++i) for (j = 0; j < WIDTH; ++j) alive_cells_rank += curr[i][j];
		pthread_mutex_unlock (&mutexalive);

		pthread_barrier_wait(&barrier); // barrier to make sure all live cell counts and board has been updated by all threads

		if (thread_num == 0) { // reduces live cell counts for all ranks and stores
			MPI_Reduce(&alive_cells_rank, &alive_cells_all[t], 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
			alive_cells_rank = 0;
		}

		// Updates ghost rows with data from other ranks

		if (thread_num == 0) {
			MPI_Irecv(upGhost, WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 1, MPI_COMM_WORLD, &request); // receives top ghost row from previous rank
			MPI_Irecv(downGhost, WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 2, MPI_COMM_WORLD, &request2); // receives down ghost row from following rank
			MPI_Isend(curr[rows_per_rank-1], WIDTH, MPI_CHAR, (mpi_rank == mpi_size - 1) ? 0 : mpi_rank + 1, 1, MPI_COMM_WORLD, &request3); // send last row to following rank
			MPI_Isend(curr[0], WIDTH, MPI_CHAR, (mpi_rank == 0) ? mpi_size - 1 : mpi_rank - 1, 2, MPI_COMM_WORLD, &request4); // receives first row to previous rank
			MPI_Wait(&request, &status);
			MPI_Wait(&request2, &status);
		}

	}

	if (thread_num != 0) pthread_exit(NULL); // if not main thread, exit
	return NULL;
}

int main(int argc, char *argv[]) {
	// Initializes variables, thread barrier, and MPI

	int i;
	unsigned short ** heatmap;
	InitDefault();
	MPI_Init( &argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	num_threads = (64 * 128) / mpi_size; // (threads_times_ranks * nodes) / mpi_size

	if (mpi_rank == 0) g_start_cycles = GetTimeBase();
	rows_per_rank = HEIGHT / mpi_size;
	rows_per_thread = rows_per_rank / num_threads;
	alive_cells_all = (mpi_rank == 0) ? (long long*)malloc(TIME * sizeof(long long)) : NULL;
	if (num_threads > 1) threadIDs = (pthread_t*)malloc((num_threads - 1) * sizeof(pthread_t));
	pthread_barrier_init(&barrier, NULL, num_threads);

	curr = (char**)malloc(rows_per_rank * sizeof(char*));
	for (i = 0; i < rows_per_rank; ++i) curr[i] = (char*)malloc(WIDTH * sizeof(char));
	upGhost = (char*)malloc(WIDTH * sizeof(char));
	downGhost = (char*)malloc(WIDTH * sizeof(char));

	// Sets the intial state of the board and ghost rows

	for (i = 0; i < rows_per_rank; ++i) memset(curr[i], 1, WIDTH);
	memset(upGhost, 1, WIDTH);
	memset(downGhost, 1, WIDTH);
	
	// Updates rows via threads (including main thread)

	MPI_Barrier( MPI_COMM_WORLD );

	for (i = 1; i < num_threads; ++i) pthread_create(&threadIDs[i-1], NULL, updateRows, (void *) (long) i); // creates threads
	updateRows((void *) (long) (i = 0));
	for (i = 0; i < num_threads - 1; ++i) pthread_join(threadIDs[i], NULL); // joins threads

	if (mpi_rank == 0) {
		g_end_cycles = GetTimeBase();
		g_time_in_secs = ((double)(g_end_cycles - g_start_cycles)) / processor_frequency;
		printf("Finished simulation in %f seconds. Number of alive cells: %lld\n", g_time_in_secs, total_alive_cells);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	//We won't need these anymore.

	//Now, let's (dumbly) gather everything in Rank 0.
	if (mpi_rank == 0){
		int j;

		//Allocate space for the heat map and for the expanded curr.
		heatmap = calloc(sizeof(short*), HEIGHT/32);
		for (i = 0; i < HEIGHT/32; ++i) heatmap[i] = calloc(sizeof(short), WIDTH/32);

		curr = (char**)realloc(curr, HEIGHT * sizeof(char*));
		for (i = rows_per_rank; i < HEIGHT; ++i) curr[i] = calloc(sizeof(char), WIDTH);
  		

  		//gather all the rows from the other ranks.
		for (i = 1; i < mpi_size; ++i){
			int row;
			//printf("0: P%d", i);
			for (row = 0; row < rows_per_rank; ++row){
				//printf("0: %d", i*rows_per_rank + row);
				MPI_Recv(curr[i*rows_per_rank + row], WIDTH, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}

		//Now that we have all our rows, construct our heat map.
		for (i = 0; i < HEIGHT; ++i){
			for (j = 0; j < WIDTH; ++j){
				heatmap[i/32][j/32] += curr[i][j];
			}
		}
		//Now draw the heatmap.
		//saveMap(heatmap);
		saveTextMap(heatmap);

	} else {
		//Give my rows to rank 0.
		int row;
		for (row = 0; row < rows_per_rank; ++row){
			MPI_Send(curr[row], WIDTH, MPI_CHAR, 0, 3, MPI_COMM_WORLD);
		}
	}



	//Cleanup time.

	MPI_Barrier(MPI_COMM_WORLD);	
		free(upGhost);
	free(downGhost);
	if (threadIDs) free(threadIDs);

	if (mpi_rank != 0) for (i = 0; i < rows_per_rank; ++i) free(curr[i]);
	else for (i = 0; i < HEIGHT; ++i) free(curr[i]);
	free(curr);
	
	MPI_Finalize();
	return 0;
	
}