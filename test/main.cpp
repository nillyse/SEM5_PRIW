#include <iostream>
#include <fstream>
#include<stdio.h>
#include<stdlib.h>
#include <omp.h>

using namespace std;

int ile = 1;
int step = 10;

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

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

void writePPM(const char *filename, PPMImage *img)
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

int mask[3][3] = {
    {1,1,1},
    {1,4,1},
    {1,1,1},
    };

void filtr(int x, int y, PPMImage *img,PPMImage *img2){
	int valuer = 0;
	int valueg = 0;
	int valueb = 0;
	
	for (int i = - 1; i < 2; i++)
        for (int j = - 1; j < 2; j++){
		
			valuer += img->data[((x + i) * img->x)+(y+j)].red * mask[i+1][j+1];
			valueg += img->data[((x + i) * img->x)+(y+j)].green * mask[i+1][j+1];
			valueb += img->data[((x + i) * img->x)+(y+j)].blue * mask[i+1][j+1];
	
		}
     img2->data[(x * img->x)+(y)].red = valuer / 12;
     img2->data[(x * img->x)+(y)].green = valueg / 12;
     img2->data[(x * img->x)+(y)].blue = valueb / 12;
}

void addFiltr(PPMImage *img,PPMImage *img2){
	int id,i,j;

	double poczatek = omp_get_wtime();
	#pragma omp parallel private(i,j,id), num_threads(ile)
	{
		id = omp_get_thread_num();
       	#pragma omp for schedule(static,step), collapse(2)
			for(i=1;i<img->x-1;i++)
				for(j=1;j<img->y-1;j++)
					filtr(i,j,img,img2);
	}
	double kn = omp_get_wtime();
	cout << "czas dla rozmiaru "<<img->x<<"x"<<img->y<<" dla "<<ile<<" watkow (static, "<<step<<"): " << kn - poczatek << "s\n";
}

int main(){
   	PPMImage *image;
   	
   	
   	
   	PPMImage *image2;
	image = readPPM("bla.ppm");
	image2 = readPPM("bla.ppm");

   	addFiltr(image,image2);
	writePPM("zdj_141.ppm",image2);
}
