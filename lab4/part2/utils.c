#include <stdlib.h>
#include <stdio.h>
#include "utils.h"


/* This function checks for failed allocations on the heap
  Suggested usage: assert(checkPointer(p, m))) */
int checkPointer(void *pointer, char *error_message) {
    if (pointer == NULL) {
        printf("%s\n", error_message);
        exit(0);
    }
    return 1;
}

/***************************
* Print to console functions
****************************/
void printStencilMatrix(StencilMatrix *stencil) {
    printf("\nStencil: \n");
    int row, col;
    for (row = 0; row < stencil->y; row++){
        for (col = 0; col < stencil->x; col++) {
            printf("Stencil[%d][%d]: %d\n", row, col, stencil->data[row][col]);
        }
    }
}


/*************************
* Log to File functions
*************************/
/*
    Logs a message in the DEBUG_FILE
*/
void logToFile(char *message)
{
    FILE* fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, message);
    fclose(fp);

}

/*
    Logs a message with int in the DEBUG_FILE
*/
void logToFileWithInt(char *formatString, int var)
{
    FILE* fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, formatString, var);
    fclose(fp);

}

/*
    Logs a message with 2 ints in the DEBUG_FILE
*/
void logToFileWithTwoInts(char *formatString, int var, int var2) {
    FILE *fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, formatString, var, var2);
    fclose(fp);
}


/*
    Logs an array of chars in the DBEUG_FILE
*/
void logCharArrayToFile(char *array, int sz, char *message) {
    int j;
    logToFile(message);
    logToFile("[ ");
    for (j = 0; j < sz; j++) {
        logToFileWithInt("%d ", array[j]);
    }
    logToFile("]\n");
}


/*
    Prints the contents of a PPImage Matrix in the
    DEBUG_FILE
*/
void logImageToFile(PPMImageMatrix *imag) {
    logToFile("About to print your image, sir\n");
    int row, col;
    logToFile("[ ");
    for (row = 0; row < imag->y; row++) {
        logToFile("[ ");
        for (col = 0; col < imag->x; col++) {
            logToFileWithInt("%d ", imag->data[row][col].red);
            logToFileWithInt("%d ", imag->data[row][col].green);
            logToFileWithInt("%d | ", imag->data[row][col].blue);
        }
        logToFile("]\n");
    }
    logToFile("]");
}