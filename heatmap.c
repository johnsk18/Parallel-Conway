#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "heatmap.h"

#define THRESHOLD 0.5

typedef struct ColorType {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Color;

Color getColor (short s) { 
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
  int width = 1024, height = 1024;
  //Dumb way of determining the filename without needing fancy logic for it.
  if (THRESHOLD == 0)         filename = "map00.ppm";
  else if (THRESHOLD == 0.25) filename = "map25.ppm";
  else if (THRESHOLD == 0.50) filename = "map50.ppm"; 
  else if (THRESHOLD == 0.75) filename = "map75.ppm";
  else filename = "map??";

  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr,"Unable to open %s for writing\n", filename);
    return 0;
  }
  // header information
  fprintf (file, "P6\n");
  fprintf (file, "1000 1000\n"); //Width and then Height, both of which will be 1000
  fprintf (file, "255\n");

  // the data
  // (0,0) is the bottom left corner this way.
  for (int y = height-1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      Color color = getColor(heatmap[x][y]);
      fputc(color.r, file);
      fputc(color.g, file);
      fputc(color.b, file);
    }
  }
  fclose(file);
  return 1;
}

int main(){

  unsigned short** map = calloc(sizeof(short*), 1000);
  for (int i = 0; i < 1000; ++i) map[i] = calloc(sizeof(short), 1000);


  for (int i = 0; i < 1000; ++i){
    for (int j = 0; j < 1000; ++j){
      map[i][j] = i;
    }
  }

  saveMap(map);

  for (int i = 0; i < 1000; ++i) free(map[i]);
  free(map);

  return 0;
}