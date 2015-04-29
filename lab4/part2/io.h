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

// Given the filepath of a jpeg file, converts it to a ppm format
// and returns the corresponding filepath
char *jpegToPPM(char *filepath);

// Read a ppm file into the PPMImage struct
PPMImage *readPPM(const char *filename);

// Write the PPMImage Struct to file as a PPM file
void writePPM(char *filename, PPMImage *img);

// Read a PPMImage struct into a PPMImageMatrix;
PPMImageMatrix *convertPPMImageToPPMImageMatrix(PPMImage *pimage);

PPMImage *convertPPMImageMatrixToPPMImage(PPMImageMatrix *pimagmatrix);

