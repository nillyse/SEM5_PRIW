#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <omp.h>

int iXmax;
int iYmax;
int t;
const double CxMin = -2.5;
const double CxMax = 1.5;
const double CyMin = -2.0;
const double CyMax = 2.0;
const int MaxColorComponentValue = 255;
const int IterationMax = 200;
const double EscapeRadius = 2;
double ER2 = EscapeRadius * EscapeRadius;
double PixelWidth;
double PixelHeight;
unsigned char RGB[9][3] = {
    {232, 58, 69},   //red  /*  initializers for row indexed by 0 */
    {207, 66, 150},  //purple
    {240, 180, 236}, //pink
    {77, 124, 219},  //blue
    {92, 218, 224},  //cyan
    {86, 232, 118},  //green
    {236, 245, 120}, //yellow   /*  initializers for row indexed by 1 */
    {240, 161, 101}, //orange /*  initializers for row indexed by 2 */
    {255, 255, 255}  //white
};

unsigned char ***color;
int *iterationCount;

void mandelbrot(int size, omp_sched_t type)
{

    int id;
    double Zx, Zy;
    double Zx2, Zy2;
    int Iteration;
    double Cx, Cy;
    int iY;
    omp_set_num_threads(t);
#pragma omp parallel private(id, Zx, Zy, Zx2, Zy2, Iteration, Cx, Cy, iY)
    {
        id = omp_get_thread_num();
        omp_set_schedule(type, size);

#pragma omp for schedule(runtime)
        for (iY = 0; iY < iYmax; iY++)
        {

            Cy = CyMin + iY * PixelHeight;
            if (fabs(Cy) < PixelHeight / 2)
                Cy = 0.0;
            for (int iX = 0; iX < iXmax; iX++)
            {
                Cx = CxMin + iX * PixelWidth;
                Zx = 0.0;
                Zy = 0.0;
                Zx2 = Zx * Zx;
                Zy2 = Zy * Zy;
                for (Iteration = 0; Iteration < IterationMax && ((Zx2 + Zy2) < ER2); Iteration++)
                {
                    Zy = 2 * Zx * Zy + Cy;
                    Zx = Zx2 - Zy2 + Cx;
                    Zx2 = Zx * Zx;
                    Zy2 = Zy * Zy;
                };
                if (Iteration == IterationMax)
                    color[iY][iX] = RGB[8];

                else
                    color[iY][iX] = RGB[id];
                iterationCount[id]++;
            }
        }
    }
}

int main(int argc, char **argv) // iXmax, iYmax, ilość wątków
{

    iXmax = atoi(argv[1]);
    iYmax = atoi(argv[1]);
    t = atoi(argv[2]);
    int block_size = atoi(argv[3]);
    char *type = argv[4];

    PixelWidth = (CxMax - CxMin) / iXmax;
    PixelHeight = (CyMax - CyMin) / iYmax;
    color = malloc(iYmax * sizeof(unsigned char **));
    iterationCount = malloc(t * sizeof(int));
    for (int i = 0; i < iXmax; i++)
    {
        color[i] = malloc(iXmax * sizeof(unsigned char *));
    }
    omp_sched_t ompScheduleType;
    if (strcmp(type, "static") == 0)
        ompScheduleType = omp_sched_static;
    else if (strcmp(type, "dynamic") == 0)
        ompScheduleType = omp_sched_dynamic;
    else
        ompScheduleType = omp_sched_guided;

    float t0 = omp_get_wtime();
    mandelbrot(block_size, ompScheduleType);
    float t1 = omp_get_wtime() - t0;

    printf("%d:%d:%d:%s:%f\n", iXmax, (int)t, block_size, type, t1); //rozmiar obrazu, ilość wątków, wielkość bloków, typ, czas
    for (int i = 0; i < t; i++)
    { //ilość iteracji per wątek
        printf("%d:%d\n", i + 1, iterationCount[i]);
    }

    FILE *fp;
    char filename[50];
    sprintf(filename, "%d_%d_%d_%s.ppm", iXmax, (int)t, block_size, type);

    fp = fopen(filename, "wb"); /* b -  binary mode */
    fprintf(fp, "P6\n # \n %d\n %d\n %d\n", iXmax, iYmax, MaxColorComponentValue);
    for (int iY = 0; iY < iYmax; iY++)
    {
        for (int iX = 0; iX < iXmax; iX++)
        {

            fwrite(color[iY][iX], 1, 3, fp);
        }
    }
    fclose(fp);
}
