#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <omp.h>

typedef struct
{
    unsigned char red, green, blue;
} PPMPixel;

typedef struct
{
    int x, y;
    PPMPixel **data;
} PPMImage;

int weights[7][7] = {
    {1, 1, 2,2,2,1,1},
    {1,2,2,4,2,2,1},
    {2,2,4,8,4,2,2},
    {2,4,8,16,8,4,2},
    {2,2,4,8,4,2,2},
    {1,2,2,4,2,2,1},
    {1, 1, 2,2,2,1,1}
    };
int sum_weights = 140;

PPMImage *image;
PPMImage *image_copy;

static PPMImage *readPPM(const char *filename)
{
    char buff[50];
    FILE *fp;
    int c, rgb_comp_color;
    //open PPM file for reading
    fp = fopen(filename, "rb");
    //alloc memory form image
    image = (PPMImage *)malloc(sizeof(PPMImage));
    image_copy = (PPMImage *)malloc(sizeof(PPMImage));

    fgets(buff, 1000, fp);
    fgets(buff, 1000, fp);
    //read image size information
    fscanf(fp, "%d %d\n", &image->x, &image->y);
    fgets(buff, 1000, fp);

    image->data = (PPMPixel **)malloc(image->x * sizeof(PPMPixel *));
    image_copy->data = (PPMPixel **)malloc(image->x * sizeof(PPMPixel *));
    for (int x = 0; x < image->x; x++)
    {
        //memory allocation for pixel data
        image->data[x] = (PPMPixel *)malloc(image->y * sizeof(PPMPixel));
        image_copy->data[x] = (PPMPixel *)malloc(image->y * sizeof(PPMPixel));
    }
    for (int y = 0; y < image->y; y++)
    {
        for (int x = 0; x < image->x; x++)
        {
            fread(&image->data[x][y], 3, 1, fp);
            image_copy->data[x][y] = image->data[x][y];
        }
    }
    fclose(fp);
    return image;
}

void calculate_sum(int positionX, int positionY)
{
    int sum_weights_red, sum_weights_green, sum_weights_blue;
    sum_weights_red = 0;
    sum_weights_green = 0;
    sum_weights_blue = 0;
    for (int i = - 3; i < 3; i++){
        for (int j = - 3; j < 3; j++){
            sum_weights_red += weights[i + 3][j + 3] * image->data[positionX + i][positionY +j].red;
            sum_weights_green += weights[i + 3][j + 3] * image->data[positionX + i][positionY +j].green;
            sum_weights_blue += weights[i + 3][j + 3] * image->data[positionX + i][positionY +j].blue;
        }
    }


    image_copy->data[positionX][positionY].red = sum_weights_red / sum_weights;
    image_copy->data[positionX][positionY].green = sum_weights_green / sum_weights;
    image_copy->data[positionX][positionY].blue = sum_weights_blue / sum_weights;
}

void LP1(int size, omp_sched_t type, int t)
{
    image_copy->x = image->x;
    image_copy->y = image->y;
    int i, j;
    omp_set_schedule(type, size);
    omp_set_num_threads(t);
#pragma omp parallel private(i, j)
    {

#pragma omp for schedule(runtime)
        for (i = 3; i < image->x - 6; i++)
        {
            for (j = 3; j < image->y - 6; j++)
            {
                calculate_sum(i, j);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    image = readPPM("image.ppm");
    int t = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    char *type = argv[3];
    omp_sched_t ompScheduleType;
    if (strcmp(type, "static") == 0)
        ompScheduleType = omp_sched_static;
    else if (strcmp(type, "dynamic") == 0)
        ompScheduleType = omp_sched_dynamic;
    else
        ompScheduleType = omp_sched_guided;

    double t0 = omp_get_wtime();
    LP1(block_size, ompScheduleType, t);
    double t1 = omp_get_wtime() - t0;

    printf("%dx%d\t%d\t%d\t%s\t%f\n", image->x, image->y, t, block_size, type, t1); //size, ilość wątków, wielkość bloków, schedule type, czas

    char filename[50];
    sprintf(filename, "%dx%d_%d_%d_%s.ppm", image->x, image->y, t, block_size, type);
    fp = fopen(filename, "wb"); /* b -  binary mode */
    fprintf(fp, "P6\n#\n%d %d\n%d\n", image->x, image->y, 255);
    for (int y = 0; y < image->y; y++)
    {
        for (int x = 0; x < image->x; x++)
        {
            fwrite(&image_copy->data[x][y], 1, 3, fp);
        }
    }

    fclose(fp);
}