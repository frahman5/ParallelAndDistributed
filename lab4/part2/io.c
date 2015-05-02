#include "io.h"

#define CREATOR "Faiyam Rahman and Oliver Hoffman"
#define RGB_COMPONENT_COLOR 255

/*******************************
* String manipulation functions
********************************/
/*
  Replaces the content of a string with the indicated 
  substring. 
*/
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

/***************************************
* Format conversion functions
***************************************/
/*
  Transforms a JPEG to PPM and returns the filename
*/
char *jpegToPPM(char *filepath) {

    char *output_file = str_replace(filepath, "jpg", "ppm");
    printf("Original: %s and New one: %s\n", filepath, output_file);

    // Write the call to jpegtopnm as a string
    char *system_call = (char *)malloc((strlen(filepath) + 30) * sizeof(char));
    checkPointer(system_call, "Failed to allocate space on heap for system_call");

    // Make the call to jpegtopnm, converting the file, and return to user    
    // sprintf(system_call, "jpegtopnm %s.jpg > %s.ppm", basename, basename);
    
    printf("HH\n");
    sprintf(system_call, "jpegtopnm %s > %s", filepath, output_file);
    printf("About to convert %s\n", system_call);
    int status = system(system_call);
    return output_file;
}

/*
  Transforms a PPM to JPEG and returns the filename.
*/
char *ppmToJPEG(char *filepath) {

    char *output_file = str_replace(filepath, ".ppm", "_filtered.jpg");

    // Write the call to jpegtopnm as a string
    char *system_call = (char *)malloc((strlen(filepath) + 200) * sizeof(char));
    checkPointer(system_call, "Failed to allocate space on heap for system_call");

    // Make the call to jpegtopnm, converting the file, and return to user    
    // sprintf(system_call, "jpegtopnm %s.jpg > %s.ppm", basename, basename);
    sprintf(system_call, "pnmtojpeg %s > %s", filepath, output_file);
    int status = system(system_call);
    return output_file;
}

/**********************
* System call functions
***********************/
/*
  Opens the image given the filepath
*/
void openImage(char *filepath)
{
    // Write the call to jpegtopnm as a string
    char *system_call = (char *)malloc((strlen(filepath) + 30) * sizeof(char));
    checkPointer(system_call, "Failed to allocate space on heap for system_call");

    // Make the call to jpegtopnm, converting the file, and return to user    
    // sprintf(system_call, "jpegtopnm %s.jpg > %s.ppm", basename, basename);
    sprintf(system_call, "open %s", filepath);
    int status = system(system_call);
}


/******************************
* Data structure manipulations
*******************************/
/*
  Transforms a PPMIMageMatrix to its equivalent PPMImage structure
*/
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

/*
  Transforms a PPMImage to its equivalent PPMImageMatrix structure
*/
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

/**********************
* Read files functions
***********************/
/*
  Reads the stencil from the PGM file
*/
StencilMatrix *readStencil(const char *filename)
{
     char buff[16];
     StencilMatrix *stencil;
     FILE *fp;
     int c, p, M;

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
    if (buff[0] != 'P' || buff[1] != '2') {
         fprintf(stderr, "Invalid stencil image format (must be 'P2')\n");
         exit(1);
    }

    //alloc memory form image
    stencil = (StencilMatrix *)malloc(sizeof(StencilMatrix));
    if (!stencil) {
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
    if (fscanf(fp, "%d %d", &stencil->x, &stencil->y) != 2) {
         fprintf(stderr, "Invalid stencil size (error loading '%s')\n", filename);
         exit(1);
    }
    logToFileWithInt("Width: %d\n", stencil->x);
    logToFileWithInt("Height: %d\n", stencil->y);

    //read M max
    if (fscanf(fp, "%d", &M) != 1) {
         fprintf(stderr, "Invalid M value (error loading '%s')\n", filename);
         exit(1);
    }
    while (fgetc(fp) != '\n');

    // Allocate space for the data in the stencil data
    stencil->data = (char **)malloc(stencil->y * sizeof(char *));
    checkPointer(stencil->data, "Failed to allocate stencil **data on the heap");
    int i;
    for(i = 0; i < stencil->y; i++){
        stencil->data[i] = (char *)malloc(stencil->x * sizeof(char));
        checkPointer(stencil->data[i], "Failed to allocate data[i] on the heap");
    }

    // Read from file and assign to stencil data
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int y = 0;
    int x = 0;

    while ((read = getline(&line, &len, fp)) != -1) 
    {
      x = 0;
      while(x < stencil->x)
      {
        sscanf(line, "%d %[^\t\n]", &p, line);
        // needs fixing 
        //stencil->data[y][x] = -4+(8*p)/(M-1); 
        stencil->data[y][x] = p; 
       // printf("%d ", p);
        ++x;        
      }
      //printf("\n");
      ++y;  
    }

    return stencil;
}

/*
  Reads the PPM image resulting from JPEG transformation
  into a PPImage data structure
*/
PPMImage *readPPM_P6(const char *filename)
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

/************************
* Write files functions
*************************/
/*
  Writes the contents of a PPMImage structure to a 
  PPM file
*/
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




