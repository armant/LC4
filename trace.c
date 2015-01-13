#include <stdlib.h>
#include <stdio.h>

#include "ObjectFiles.h"

int main(int argc, char **argv) {
  MachineState *state;
  int run, i, readSuccess;
  unsigned short int prevPC;
  FILE *outputFile;

  // Open up a file for binary writing
  outputFile = fopen (argv[1], "wb");
  
  state = malloc(sizeof(*state));
  Reset(state);

  readSuccess = 0;

  for (i = 2; i < argc; i++) {
    readSuccess = 1;
    run = ReadObjectFile(argv[i], state);

    if (run != 0) {
      readSuccess = 0;
      break;
    }

  }

  if (argc <= 2) {
    printf("Please enter the input file name(s).\n");
  }

  if (readSuccess != 0) {
  
    do {
      prevPC = state -> PC;
      run = UpdateMachineState(state);

      if (run != 1) {
        fwrite(&prevPC, sizeof(unsigned short int), 1, outputFile);
        fwrite(&state -> memory[prevPC], sizeof(unsigned short int), 1, 
            outputFile);
      }

    } while (run == 0); // & i < 31050
  
  }

  fclose (outputFile);
  free(state);
  return 0;
}