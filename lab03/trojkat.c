#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

int length;
int generations_max;
int t = 3;
struct information
{
    int gen;
    int x;
    int y;
};

unsigned char RGB[5][3] = {
    {0, 0, 0},
    {42, 140, 68},  //green  /*  initializers for row indexed by 0 */
    {42, 78, 140},  //blue
    {140, 42, 70},  //red
    {242, 235, 237} //white
};
unsigned char ***color;

void *triangle(void *inf)
{
    int x_start, x_end, y_start, y_end;
    struct information *info = (struct information *)inf;
    int len = length >> (info->gen-1);
    struct information *child_info = malloc(t * sizeof(struct information));
    if (info->gen >= generations_max)
    {
        pthread_exit(NULL);
    }
    else
    {
        x_start = info->x;
        y_start = info->y;
        for (int i = y_start; i < y_start + len; i++){
            color[x_start][i] = RGB[3];
        }

        for (int i = x_start; i < x_start + len; i++){
            color[i][y_start + len-1] = RGB[3];
        }
        int j = y_start;
        for (int i = x_start; i < x_start + len; i++, j++){
            color[i][j] = RGB[3];
        }




        for (int i = 0; i < t; i++)
        {

            child_info[i].gen = info->gen + 1;
        }
        child_info[0].x = x_start;
        child_info[0].y = y_start;
        child_info[1].x = x_start;
        child_info[1].y = y_start + len/2;
        child_info[2].x = x_start + len/2;
        child_info[2].y = y_start + len/2;
        pthread_t threads[t];
        for (int i = 0; i < t; i++)
        {
            pthread_create(&threads[i], NULL, triangle, &child_info[i]);
        }

        for (int i = 0; i < t; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
}

int main(int argc, char **argv)
{
    //length = atoi(argv[1]);
    //generations_count = atoi(argv[2]);
    length = 1600;
    generations_max = 8;
    color = malloc(length * sizeof(unsigned char **));
    for (int iY = 0; iY < length; iY++)
    {
        color[iY] = malloc(length * sizeof(unsigned char *));
    }
    for (int iY = 0; iY < length; iY++)
    {
        for (int iX = 0; iX < length; iX++)
        {
            color[iY][iX] = RGB[4];
        }
    }
    struct information *info = malloc(sizeof(struct information));
    info->x = 0;
    info->y = 0;
    info->gen = 1;
    triangle(info);

    //char *filename = "trojkat.png";
    FILE *fp = fopen("trojkat.ppm", "wb"); /* b -  binary mode */
    fprintf(fp, "P6\n # \n %d\n %d\n %d\n", length, length, 255);
    //fprintf(fp, "P6\n # \n %d\n %d\n %d\n", iXmax, iYmax, MaxColorComponentValue);
    for (int iY = 0; iY < length; iY++)
    {
        for (int iX = 0; iX < length; iX++)
        {

            fwrite(color[iX][iY], 1, 3, fp);
        }
    }
    fclose(fp);

    return 0;
}