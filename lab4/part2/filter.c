#include "filter.h"

void applyStencilToOnePixel(
        PPMImageMatrix *new_pmag, 
        PPMImageMatrix *pmag, 
        int imag_row_center, 
        int imag_col_center, 
        StencilMatrix *stencil 
    ) {
    
    // Make sure we have an odd by odd stencil
    assert(stencil->x % 2 == 1);
    assert(stencil->y % 2 == 1);

    // Find the center of the stencil
    int stencil_centerrow = ((stencil->y + 1) / 2) - 1;
    int stencil_centercol = ((stencil->x + 1) / 2) - 1;

    // Find the new value of the pixel
    int stencil_row, stencil_col, image_row, image_col;
    int val_red = 0;
    int val_green = 0;
    int val_blue = 0;
    for(stencil_row = 0; stencil_row < stencil->y; stencil_row++) {
        for(stencil_col = 0; stencil_col < stencil->x; stencil_col++) {
            
            // Find the pixel in the original image that needs to be multipled
            // by the current pixel of the stencil
            image_row = imag_row_center - stencil_centerrow + stencil_row;
            image_col = imag_col_center - stencil_centercol + stencil_col;

            // If that pixel exists (i.e is nonnegative) in the original image, 
            // update the values of red blue and green
            if ((image_row >= 0) && (image_col >= 0) && (image_row < pmag->y) && (image_col < pmag->x)) {

                val_red += stencil->data[stencil_row][stencil_col] * (pmag->data[image_row][image_col]).red;
                val_green += stencil->data[stencil_row][stencil_col] * (pmag->data[image_row][image_col]).green;
                val_blue += stencil->data[stencil_row][stencil_col] * (pmag->data[image_row][image_col]).blue;
            } 

        }
    }
    new_pmag->data[imag_row_center][imag_col_center].green = val_green;
    new_pmag->data[imag_row_center][imag_col_center].blue = val_blue;
    new_pmag->data[imag_row_center][imag_col_center].red = val_red;
}

PPMImageMatrix *copyImageMatrix(PPMImageMatrix *og_pmag) {
    PPMImageMatrix *copy_pmag = (PPMImageMatrix *)malloc(sizeof(PPMImageMatrix));
    checkPointer(copy_pmag, "Failed to allocate copy_pmag in copyImageMatri");
    logToFileWithInt("\n\n***While creating copy_pmag, we allocate %d number of rows in the data**\n\n", og_pmag->y);
    copy_pmag->data = (PPMPixel **)malloc(og_pmag->y * sizeof(PPMPixel *));
    checkPointer(copy_pmag->data, "Failed to allocate to allocate copy_pmag->data in copyImageMatrix");

    copy_pmag->x = og_pmag->x;
    copy_pmag->y = og_pmag->y;
    int row, col;
    for (row = 0; row < copy_pmag->y; row++) {
        copy_pmag->data[row] = (PPMPixel *)malloc(copy_pmag->x * sizeof(PPMPixel));
        checkPointer(copy_pmag->data[row], "failled to allocate a row in copyImageMatrix\n");
        for(col = 0; col < copy_pmag->x; col++) {
            copy_pmag->data[row][col] = og_pmag->data[row][col];
        }
    }
    logToFileWithTwoInts("Copy Pmag (x, y): (%d, %d)\n", copy_pmag->x, copy_pmag->y);
    return copy_pmag;
}

void freePPMImageMatrix(PPMImageMatrix *pmag) {
    int row, col;
    for (row = 0; row < pmag->y; row++) {
        free(pmag->data[row]);
    }
    free(pmag->data);
    free(pmag);
}


void checkCopy(PPMImageMatrix *pmag1, PPMImageMatrix *pmag2) {
    int row, col;
    assert (pmag1->x == pmag2->x);
    assert (pmag1->y == pmag2->y);
    for (row = 0; row < pmag1->y; row++) {
        for (col = 0; col < pmag2->x; col++) {
            assert (checkEqualPixel2(pmag1->data[row][col], pmag2->data[row][col]));
        }
    }
    logToFile("\n\n\n\n ****** The original and copy Image Matrices are both the same, yo! ***** \n\n\n\n\n");
}

PPMImageMatrix *applyStencil(PPMImageMatrix *pmag, StencilMatrix *stencil) {
    int row, col;
    PPMImageMatrix *updated_pmag = copyImageMatrix(pmag);
    checkCopy(updated_pmag, pmag);

    // logImageToFile(updated_pmag);

    // PPMPixel *new_pixel = (PPMPixel *)malloc(sizeof(PPMPixel));
    // checkPointer(new_pixel, "Failed to allocate new_pixel in applyStencil\n");
    for (row = 0; row < pmag->y; row++) {
        for (col = 0; col < pmag->x; col++) {
            logToFileWithTwoInts("From applyStencil: Applying stencil to pixel (%d, %d)\n",  row, col);

            //Check if changes in updated image are correct
            applyStencilToOnePixel(updated_pmag, pmag, row, col, stencil);
        
            // updated_pmag->data[row][col].red = new_pixel->red;
            // updated_pmag->data[row][col].blue = new_pixel->blue;
            // updated_pmag->data[row][col].green = new_pixel->green;
        }
    }
    // printf("About to print 0,0 red pixel data\n");
    // printf("0,0 red of pmag data: %d\n", pmag->data[0][0].red);
    // printf("0,0 red of updated_pmag data: %d\n", updated_pmag->data[0][0].red);
    freePPMImageMatrix(pmag);
    return updated_pmag;
}

int checkEqualPixel2(PPMPixel pixel1, PPMPixel pixel2) {

    return (pixel1.red == pixel2.red) && 
           (pixel1.blue == pixel2.blue) &&
           (pixel1.green == pixel2.green);
}

int checkConversion2(PPMImage *pimag, PPMImageMatrix *pimagmatrix) {
    int i;
    
    for (i = 0; i < (pimag->x * pimag->y); i++) {
        assert(checkEqualPixel2(pimag->data[i], pimagmatrix->data[i / pimag->x][i % pimag->x]) );
    }

    return 1;
}

// Expected use: ./filter image.jpg stencil.pgm num_threads num_repetitions_of_stencil
int main(int argc, char **argv) {

    // Clean the debug log1
    FILE *fp = fopen("debug.txt", "w+");
    fclose(fp);

    // Take in jpg image, convert it to a ppmimagematrix struct
    char *ppm_file = jpegToPPM(argv[1]);
    PPMImage *pimag = readPPM(ppm_file);
    PPMImageMatrix *pimagmatrix = convertPPMImageToPPMImageMatrix(pimag);
    checkConversion2(pimag, pimagmatrix);

    // Apply the fixed, stupid ass stencil
    char data[5][3] = { {4,   -2,  3}, 
                        {1,   0,   4}, 
                        {-2,  3,   1},
                        {0,   4,   3},
                        {-4,  -3,  -2}  };
        // Allocate and set the stencil
    StencilMatrix *stencil = (StencilMatrix *)malloc(sizeof(StencilMatrix));
    checkPointer(stencil, "stencil couldn't allocate in main function!");
    stencil->x = 3; stencil->y = 5;
    stencil->data = (char **)malloc(stencil->y * sizeof(char *));
    checkPointer(stencil->data, "Failed to allocate stencil->data in main function");
    
    int row, col;
    for (row = 0; row < stencil->y; row++) {
        stencil->data[row] = (char *)malloc(stencil->x * sizeof(char));
        checkPointer(stencil->data[row], "Failed to allocate stencil->data[row] in main function\n");
        for (col = 0; col < stencil->x; col++) {
            printf("Creating stencil. (Row, Col): (%d, %d)\n", row, col);
            stencil->data[row][col] = data[row][col];
        }
    }
    printStencilMatrix(stencil);
    pimagmatrix = applyStencil(pimagmatrix, stencil);

    // Take the resultant image matrix structure and save it to file as a PPM
    PPMImage *filtered_image = convertPPMImageMatrixToPPMImage(pimagmatrix);
    checkConversion2(filtered_image, pimagmatrix);
    printf("Argv1: %s\n", argv[1]);
    printf("Output image name: %s\n", getOutputImageName(argv[1], "ppm") );
    // writePPM(getOutputImageName(argv[1], ".ppm"), filtered_image);

    // Take the resultant image, convert it to jpg
    // ppmToJPEG

    // Open the file

}