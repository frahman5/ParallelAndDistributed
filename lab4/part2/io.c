#include "io.h"

#define CREATOR "Faiyam Rahman"
#define RGB_COMPONENT_COLOR 255


char *getOutputImageName(char *filepath, char *desired_ending) {

    printf("When we enter getOutputImageName, filepath, desired_ending is %s, %s\n", filepath, desired_ending);


    // Find the index of the . in filepath
    char *dot = strchr(filepath, '.');
    int index = (int)(dot - filepath);

    printf("%d index", index);

    // Copy the basename into a stirng
    char *output_file = (char *)malloc( (index + 2 + strlen(desired_ending)) * sizeof(char));
    checkPointer(output_file, "Failed to allocate memory on the heap for output_file");

    // Concatenate on the desired ending and return the string
    printf("gonna do memcpy");
    memcpy(output_file, &filepath, index + 1); // output_file = small_colorful_image
    printf("gonna put a null ending");
    *(output_file + index + 1) = '\0';
    printf("gonna do a strcat\n");
    strcat(output_file, desired_ending);

    return output_file;
}
char *jpegToPPM(char *filepath) {

    char *output_file = getOutputImageName(filepath, "ppm");

    // Write the call to jpegtopnm as a string
    char *system_call = (char *)malloc((strlen(filepath) + 30) * sizeof(char));
    checkPointer(system_call, "Failed to allocate space on heap for system_call");

    // Make the call to jpegtopnm, converting the file, and return to user    
    // sprintf(system_call, "jpegtopnm %s.jpg > %s.ppm", basename, basename);
    sprintf(system_call, "jpegtopnm %s > %s.ppm", filepath, output_file);
    int status = system(system_call);
    return output_file;
}

PPMImage *convertPPMImageMatrixToPPMImage(PPMImageMatrix *pimagmatrix) {

    // Allocate the PPMImage
    PPMImage *pimage = (PPMImage *)malloc(sizeof(PPMImage));
    checkPointer(pimage, "Failed to allocate PPMImage on the heap\n");

    // Tell the PPMImage its size
    pimage->x = pimagmatrix->x;
    pimage->y = pimagmatrix->y;

    // Allocate space for the data in the PPMImage
    pimage->data = (PPMPixel *)malloc(pimage->x * pimage->y * sizeof(PPMPixel));
    checkPointer(pimage->data, "Failed to allocate PPMPixel *data on the heap\n");

    // Iterate through the PPMImageMatrix and fill in the data
    int row, col;
    int i = 0;
    for (row = 0; row < pimage->y; row++) {
        for (col = 0; col < pimage->x; col++) {
            pimage->data[i++] = pimagmatrix->data[row][col];
        }
    }

    // Return the ImageMatrix
    return pimage;
}

PPMImageMatrix *convertPPMImageToPPMImageMatrix(PPMImage *pimage) {

    // Allocate the image matrix 
    PPMImageMatrix *pimagmatrix = (PPMImageMatrix *)malloc(sizeof(PPMImageMatrix));
    checkPointer(pimagmatrix, "Failed to allocate PPImageMatrix on the heap");

    // Tell the image matrix its size
    pimagmatrix->x = pimage->x;
    pimagmatrix->y = pimage->y;

    // Allocate space for the data in the image matrix
    pimagmatrix->data = (PPMPixel **)malloc(pimagmatrix->y * sizeof(PPMPixel *));
    checkPointer(pimagmatrix->data, "Failed to allocate PPMPixel **data on the heap");
    int i;
    for(i = 0; i < pimagmatrix->y; i++){
        pimagmatrix->data[i] = (PPMPixel *)malloc(pimagmatrix->x * sizeof(PPMPixel));
        checkPointer(pimagmatrix->data[i], "Failed to allocate data[i] on the heap");
    }

    // Iterate through the PPMImage and fill in the image matrix data
    int row, column;
    for(i = 0; i < pimagmatrix->x * pimagmatrix->y; i++) {
        row = i / pimagmatrix->x; // relies on the fact that i and row are ints, so this truncates to floor(i/x)
        column = i % pimagmatrix->x;
        pimagmatrix->data[row][column] = pimage->data[i];
    }

    // Return the Image Matrix
    return pimagmatrix;
}


PPMImage *readPPM(const char *filename)
{
     char buff[16];
     PPMImage *img;
     FILE *fp;
     int c, rgb_comp_color;

     //open PPM file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     //read image format
     if (!fgets(buff, sizeof(buff), fp)) {
          perror(filename);
          exit(1);
     }

    //check the image format
    logToFile(buff);
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
    while (getc(fp) != '\n') ;
         c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
         fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
         exit(1);
    }
    logToFileWithInt("Width: %d\n", img->x);
    logToFileWithInt("Height: %d\n", img->y);

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }
    // logToFileWithInt("RGB Comp Color: %d\n", rgb_comp_color);

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }
    // logCharArrayToFile((char *)img->data, 3 * img->x * img->y, "Pixel values for image\n");

    fclose(fp);
    return img;
}
void writePPM(char *filename, PPMImage *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
         fprintf(stderr, "Unable to open file '%s'\n", filename);
         exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments
    fprintf(fp, "# Created by %s\n",CREATOR);

    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

// void changeColorPPM(PPMImage *img)
// {
//     int i;
//     if(img){

//          for(i=0;i<img->x*img->y;i++){
//               img->data[i].red=RGB_COMPONENT_COLOR-img->data[i].red;
//               img->data[i].green=RGB_COMPONENT_COLOR-img->data[i].green;
//               img->data[i].blue=RGB_COMPONENT_COLOR-img->data[i].blue;
//          }
//     }
// }

// int main(int argc, char **argv){
//     char *ppm_file = jpegToPPM(argv[1]);
//     PPMImage *pimag = readPPM(ppm_file);

//     // Convert the
//     // Initalize the libnetpbm program
//     // PPMImage *image;
//     // image = readPPM("feep.ppm");
//     // changeColorPPM(image);
//     // writePPM("new_feep.ppm",image);
//     // printf("Press any key...");
//     // getchar();
// }
