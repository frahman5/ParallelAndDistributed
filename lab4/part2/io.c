#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CREATOR "Faiyam Rahman"
#define RGB_COMPONENT_COLOR 255

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

// Given the filepath of a jpeg file, converts it to a ppm format
// and returns the corresponding filepath
char *jpegToPPM(char *filepath) {

    // Fully determine the names of the input and output files
    char *basename = strtok(filepath, ".");
    char *input_file = (char *)malloc((strlen(basename) + 5) * sizeof(char));
    if (!input_file) {
        printf("Failed to allocate space on heap for input_file");
        exit(1);
    }
    char *output_file = (char *)malloc((strlen(basename) + 5) * sizeof(char));
    if (!output_file) {
        printf("Failed to allocate memory on the heap for output_file");
        exit(1);
    }
    sprintf(input_file, "%s.jpg", basename);
    sprintf(output_file, "%s.ppm", basename);

    // Write the call to jpegtopnm as a string
    char *system_call = (char *)malloc((strlen(filepath) + 30) * sizeof(char));
    if (!system_call) {
        printf("Failed to allocate space on heap for system_call");
        exit(1);
    }

    // Make the call to jpegtopnm, converting the file, and return to user    
    sprintf(system_call, "jpegtopnm %s.jpg > %s.ppm", basename, basename);
    int status = system(system_call);
    return output_file;
}



static PPMImage *readPPM(const char *filename)
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

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

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

    fclose(fp);
    return img;
}
// void writePPM(const char *filename, PPMImage *img)
// {
//     FILE *fp;
//     //open file for output
//     fp = fopen(filename, "wb");
//     if (!fp) {
//          fprintf(stderr, "Unable to open file '%s'\n", filename);
//          exit(1);
//     }

//     //write the header file
//     //image format
//     fprintf(fp, "P6\n");

//     //comments
//     fprintf(fp, "# Created by %s\n",CREATOR);

//     //image size
//     fprintf(fp, "%d %d\n",img->x,img->y);

//     // rgb component depth
//     fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

//     // pixel data
//     fwrite(img->data, 3 * img->x, img->y, fp);
//     fclose(fp);
// }

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

int main(int argc, char **argv){
    char *ppm_file = jpegToPPM(argv[1]);
    PPMImage *pimag = readPPM(ppm_file);

    // Convert the
    // Initalize the libnetpbm program
    // PPMImage *image;
    // image = readPPM("feep.ppm");
    // changeColorPPM(image);
    // writePPM("new_feep.ppm",image);
    // printf("Press any key...");
    // getchar();
}
