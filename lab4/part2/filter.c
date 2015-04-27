#include "filter.h"

void applyStencilToOnePixel(PPMPixel **data, int imag_row_center, int imag_col_center, StencilMatrix *stencil, 
    PPMPixel *new_pixel, int max_height, int max_width) {
    printf("Applying stencil to image pixel (%d, %d)\n", imag_row_center, imag_col_center);
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
            if ((image_row >= 0) && (image_col >= 0) && (image_row <= max_height) && (image_col <= max_width)) {

                val_red += stencil->data[stencil_row][stencil_col] * data[image_row][image_col].red;
                val_green += stencil->data[stencil_row][stencil_col]; //* data[image_row][image_col]->green;
                val_blue += stencil->data[stencil_row][stencil_col]; //* data[image_row][image_col]->blue;
            } 

        }
    }
    new_pixel->red = val_red;
    new_pixel->green = val_green;
    new_pixel->blue = val_blue;


}

PPMImageMatrix *copyImageMatrix(PPMImageMatrix *og_pmag) {
    PPMImageMatrix *copy_pmag = (PPMImageMatrix *)malloc(sizeof(PPMImageMatrix));
    checkPointer(copy_pmag, "Failed to allocate copy_pmag in copyImageMatri");
    copy_pmag->data = (PPMPixel **)malloc(copy_pmag->y * sizeof(PPMPixel *));
    checkPointer(copy_pmag->data, "Failed to allocate to allocate copy_pmag->data in copyImageMatrix");

    copy_pmag->x = og_pmag->x;
    copy_pmag->y = og_pmag->y;
    int row, col;
    for (row = 0; row < copy_pmag->y; row++) {
        // logToFileWithInt("Creating row %d in ImageMatrix\n", row);
        copy_pmag->data[row] = (PPMPixel *)malloc(copy_pmag->x * sizeof(PPMPixel));
        checkPointer(copy_pmag->data[row], "failled to allocate a row in copyImageMatrix\n");
        for(col = 0; col < copy_pmag->x; col++) {
            copy_pmag->data[row][col] = og_pmag->data[row][col];
        }
    }
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

PPMImageMatrix *applyStencil(PPMImageMatrix *pmag, StencilMatrix *stencil) {
    int row, col;
    PPMImageMatrix *updated_pmag = copyImageMatrix(pmag);
    logImageToFile(updated_pmag);

    PPMPixel *new_pixel = (PPMPixel *)malloc(sizeof(PPMPixel));
    checkPointer(new_pixel, "Failed to allocate new_pixel in applyStencil\n");
    for (row = 0; row < pmag->y; row++) {
        for (col = 0; col < pmag->x; col++) {
            printf("Row: %d, Col: %d\n",row, col);

            applyStencilToOnePixel(updated_pmag->data, row, col, stencil, new_pixel, (pmag->y)-1, (pmag->x)-1);
            // updated_pmag->data[row][col].red = new_pixel->red;
            // updated_pmag->data[row][col].green = new_pixel->green;
            // updated_pmag->data[row][col].blue = new_pixel->blue;
        }
    }
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
            stencil->data[row][col] = data[row][col];
        }
    }
    printStencilMatrix(stencil);
    pimagmatrix = applyStencil(pimagmatrix, stencil);

    // Take the resultant image, convert it to jpg

    // Open the file

}