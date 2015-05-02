#include "filter.h"

/*
    Frees a PPMImageMatrix from memory
*/
void freePPMImageMatrix(PPMImageMatrix *pmag) {
    int row, col;
    for (row = 0; row < pmag->y; row++) {
        free(pmag->data[row]);
    }
    free(pmag->data);
    free(pmag);
}

/*
    Verifies that 2 PPMImageMatrices are equal
*/
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


/*
    Checks if 2 pixels are equal
*/
int checkEqualPixel2(PPMPixel pixel1, PPMPixel pixel2) {

    return (pixel1.red == pixel2.red) && 
           (pixel1.blue == pixel2.blue) &&
           (pixel1.green == pixel2.green);
}

/*
    Checks if the contents of PPMImage are equal to the ones of a PPMIMageMatrix
*/
int checkConversion2(PPMImage *pimag, PPMImageMatrix *pimagmatrix) {
    int i;
    
    for (i = 0; i < (pimag->x * pimag->y); i++) {
        assert(checkEqualPixel2(pimag->data[i], pimagmatrix->data[i / pimag->x][i % pimag->x]) );
    }

    return 1;
}

/*
    Copies an imageMatrix into a new one and returns the new one
*/
PPMImageMatrix *copyImageMatrix(PPMImageMatrix *og_pmag) 
{
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

/*
    Applies the stencil provided to the pixel in row and col center.
    Receives the original version of the
    image and returns the updated pixel value. 
    Does not modify the original image.
*/  
int getRealRGB(float valRGB, int ksum)
{
    if (ksum <= 0)
    {
        ksum = 1;
    }
    valRGB /= ksum;

    if(valRGB > 255)
    {
        valRGB = 255;
    } 
    else if(valRGB < 0)
    {
        valRGB = 0;
    }
    return (int) valRGB;
}

PPMPixel *applyStencilToOnePixel(
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

    int sum = 0;
    for(stencil_row = 0; stencil_row < stencil->y; stencil_row++) {
        for(stencil_col = 0; stencil_col < stencil->x; stencil_col++) {
            
            // Find the pixel in the original image that needs to be multipled
            // by the current pixel of the stencil
            image_row = imag_row_center - stencil_centerrow + stencil_row;
            image_col = imag_col_center - stencil_centercol + stencil_col;

            // If that pixel exists (i.e is nonnegative) in the original image, 
            // update the values of red blue and green
            if ((image_row >= 0) && (image_col >= 0) && (image_row < pmag->y) && (image_col < pmag->x)) {

                val_red += (stencil->data[stencil_row][stencil_col]) * (pmag->data[image_row][image_col]).red;
                val_green += (stencil->data[stencil_row][stencil_col]) * (pmag->data[image_row][image_col]).green;
                val_blue += (stencil->data[stencil_row][stencil_col]) * (pmag->data[image_row][image_col]).blue;
            } 

            sum += stencil->data[stencil_row][stencil_col];
        }
    }

    PPMPixel *new_pixel = (PPMPixel *)malloc(sizeof(PPMPixel));
    checkPointer(new_pixel, "Failed to allocate new_pixel in applyStencil\n");
    new_pixel->green = getRealRGB((float)val_green, sum);
    new_pixel->blue = getRealRGB((float)val_blue, sum);
    new_pixel->red = getRealRGB((float)val_red, sum);

    return new_pixel;
}

/*
    Applies aa stencil to the entire image by iterating through each pixel
*/
PPMImageMatrix *applyStencil(PPMImageMatrix *pmag, StencilMatrix *stencil, int number_threads) {
    
    
    PPMImageMatrix *updated_pmag = copyImageMatrix(pmag);
    // logImageToFile(updated_pmag);

    #pragma omp parallel shared (updated_pmag, pmag, stencil) num_threads(number_threads)
    {
        int row, col, temp;
        int i = 0;
        PPMPixel *new_pixel = (PPMPixel *)malloc(sizeof(PPMPixel));
        checkPointer(new_pixel, "Failed to allocate new_pixel in applyStencil\n");

        #pragma omp for schedule(static)
        for(i = 0; i < pmag->x * pmag->y; i++) {
            row = i / pmag->x; // relies on the fact that i and row are ints, so this truncates to floor(i/x)
            col = i % pmag->x;
                //logToFileWithTwoInts("From applyStencil: Applying stencil to pixel (%d, %d) from thread %d\n",  row, col, omp_get_thread_num());
            //printf("From applyStencil: Applying stencil to pixel (%d, %d) from thread %d\n",  row, col, omp_get_thread_num());
               // scanf("%d", &temp);
            //Check if changes in updated image are correct
            new_pixel = applyStencilToOnePixel(pmag, row, col, stencil);
            updated_pmag->data[row][col].red = new_pixel->red;
            updated_pmag->data[row][col].blue = new_pixel->blue;
            updated_pmag->data[row][col].green = new_pixel->green;
        }
    }
    
    freePPMImageMatrix(pmag);
    return updated_pmag;
}

// Expected use: 
// gcc -o filter -fopenmp filter.c io.c utils.c
// ./filter image.jpg stencil.pgm num_threads num_repetitions_of_stencil
int main(int argc, char **argv) {


    if (argc == 5)
    {
        char *original_image = argv[1];
        char *stencil_file = argv[2];
        int num_threads = (int)strtol(argv[3],NULL, 0);
        int stencil_reps = (int)strtol(argv[4],NULL, 0);

        printf("****** Parameters received *****\n");
        printf("Image to filter: %s\n", original_image);
        printf("Filter stencil: %s\n", stencil_file);
        printf("Number of threads: %d\n", num_threads);
        printf("Stencil Repetitions: %d\n", stencil_reps);


        // Read the stencil
        StencilMatrix *stencil = readStencil(argv[2]);
        printStencilMatrix(stencil);
        
        // Clean the debug log1
        FILE *fp = fopen("debug.txt", "w+");
        fclose(fp);
    
        // Take in jpg image, convert it to a ppmimagematrix struct
        char *ppm_file = jpegToPPM(argv[1]);
        PPMImage *pimag = readPPM_P6(ppm_file);

        PPMImageMatrix *pimagmatrix = convertPPMImageToPPMImageMatrix(pimag);
        checkConversion2(pimag, pimagmatrix);

        // Apply the stencil
        clock_t begin, end;
        double time_spent;
        begin = clock();
        int i = 0;
        for (i = 0; i < stencil_reps; ++i)
        {
            pimagmatrix = applyStencil(pimagmatrix, stencil, num_threads);
        }
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Time spent for %d threads = %.11f seconds\n", num_threads,time_spent);

        // Take the resultant image matrix structure and save it to file as a PPM
        PPMImage *filtered_image = convertPPMImageMatrixToPPMImage(pimagmatrix);
        checkConversion2(filtered_image, pimagmatrix);
        writePPM(ppm_file, filtered_image);

        // Take the resultant image, convert it to jpg
        char *jpg_file = ppmToJPEG(ppm_file);
    
        // Open the FILE
        openImage(jpg_file);
        openImage(original_image);

    }
    else
    {
        printf("Invalid number of arguments\n");
    }

}
    
    /*
        // Stencil Data
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
        */
