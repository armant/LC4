/*
 * ObjectFiles.c
 */
#include <stdlib.h>
#include <stdio.h>

#include "ObjectFiles.h"



// Read an object file and modify the machine state accordingly
// Return a zero if successful and a non-zero error code if you encounter a problem
int ReadObjectFile (char *filename, MachineState *theMachineState) {
  unsigned char p1, p2;
  unsigned short int code, startAddress, n;
  int i;

  FILE *theFile;

  // Open up a file for binary reading
  theFile = fopen (filename, "rb");

  if (theFile == NULL) {
    printf ("Couldn't open file\n");
    return 1;
  }
  
  // Read the bytes in the file as integers
  // note fread returns the number of elements successfully read
  while (fread(&p1, sizeof(unsigned char), 1, theFile) == 1) {
  	fread(&p2, sizeof(unsigned char), 1, theFile);
  	code = (unsigned short int) (p1 << 8) + p2;

  	if (code == 0xCADE) {
      fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  startAddress = (unsigned short int) (p1 << 8) + p2;
  	  fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  n = (unsigned short int) (p1 << 8) + p2;

  	  for (i = 0; i < n; i++) {
  	  	fread(&p1, sizeof(unsigned char), 1, theFile);
      	fread(&p2, sizeof(unsigned char), 1, theFile);
  	  	theMachineState -> memory[startAddress + i] = 
  	  		(unsigned short int) (p1 << 8) + p2;
  	  }

    } else if (code == 0xDADA) {
      fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  startAddress = (unsigned short int) (p1 << 8) + p2;
  	  fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  n = (unsigned short int) (p1 << 8) + p2;

  	  for (i = 0; i < n; i++) {
  	  	fread(&p1, sizeof(unsigned char), 1, theFile);
      	fread(&p2, sizeof(unsigned char), 1, theFile);
  	  	theMachineState -> memory[startAddress + i] = 
  	  		(unsigned short int) (p1 << 8) + p2;
  	  }

    } else if (code == 0xC3B7) {
      fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  startAddress = (unsigned short int) (p1 << 8) + p2;
  	  fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  n = (unsigned short int) (p1 << 8) + p2;

  	  for (i = 0; i < n; i++) {
  	  	fread(&p1, 1, 1, theFile);
  	  }

    } else if (code == 0xF17E) {
  	  fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
  	  n = (unsigned short int) (p1 << 8) + p2;

  	  for (i = 0; i < n; i++) {
  	  	fread(&p1, 1, 1, theFile);
  	  }

    } else if (code == 0x715E) {
  	  
  	  fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);
	
	    fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);

	    fread(&p1, sizeof(unsigned char), 1, theFile);
      fread(&p2, sizeof(unsigned char), 1, theFile);

    }
  
  }

  fclose (theFile);
  return 0;
}