#define DEBUG_FILE              "debug.txt"
#include "io.h"

int checkPointer(void *pointer, char *error_message);

void logToFile(char *message);

void logToFileWithInt(char *formatString, int var);

// Prints an Array with an associated message
void logCharArrayToFile(char *array, int sz, char *message);

void printStencilMatrix(StencilMatrix *stencil);

void logImageToFile(PPMImageMatrix *imag);

void logToFileWithTwoInts(char *formatString, int var, int var2);

// int checkConversion(PPMImage *pimag, PPMImageMatrix *pimagmatrix);