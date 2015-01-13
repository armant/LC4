/*
 * ObjectFiles.h
 */

#include "LC4.h"

// Read an object file and modify the machine state accordingly
// Return a zero if successful and a non-zero error code if you encounter a problem
int ReadObjectFile (char *filename, MachineState *theMachineState);