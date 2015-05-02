#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Data structures to hold a PPM image
typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

typedef struct {
    int x, y;
    PPMPixel **data;
} PPMImageMatrix;

// Typedef to hold a stencil as a struct
typedef struct {
     int x, y;
     char **data;
} StencilMatrix;

/***************************************
* Format conversion functions
***************************************/

// Transforms a JPEG image into ppm P6 format and returns the file name
char *jpegToPPM(char *filepath);

// Transforms a ppm P6 image into JPEG format and returns the file name
char *ppmToJPEG(char *filepath);

/**********************
* System call functions
***********************/

// Makes the system call to open an image
void openImage(char *filepath);

/*******************************
* String manipulation functions
********************************/

// Auxiliary function to replace filenames
char *str_replace ( const char *string, const char *substr, const char *replacement );

/**********************
* Read files functions
***********************/

// Read a ppm file into the PPMImage struct
PPMImage *readPPM_P6(const char *filename);

// Reads the stencil from a pgm file and returns the correspondign struct
StencilMatrix *readStencil(const char *filename);

// Write the PPMImage Struct to file as a PPM file
void writePPM(char *filename, PPMImage *img);

/******************************
* Data structure manipulations
*******************************/

// Read a PPMImage struct into a PPMImageMatrix;
PPMImageMatrix *convertPPMImageToPPMImageMatrix(PPMImage *pimage);

// Read a PPMMatrix struct into a PPIMage
PPMImage *convertPPMImageMatrixToPPMImage(PPMImageMatrix *pimagmatrix);



