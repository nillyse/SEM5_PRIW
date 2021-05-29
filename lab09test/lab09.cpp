#include <iostream>
#include <cmath>
#include <omp.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

int size = 500;
char ***pixels;
int **iterationCount;
class Lapunov
{
public:
    char Sequence[16];
    const int noColors = 255;
    float MinR, MaxR, MinX, MaxX;
    char NoSeq = 16;
    float LapMin = -2;
    int sample;

    int DecToBin(int Dec)
    {
        int pow;
        char rest;
        pow = 65536;
        rest = 0;

        while (rest == 0 && pow > 0)
        {
            rest = (char)floor(Dec / pow);
            if (rest == 0)
                pow = floor(pow / 2);
        }
        while (pow > 1)
        {
            Dec = Dec - pow * rest;
            pow = floor(pow / 2);
            rest = (char)floor(Dec / pow);
            NoSeq++;
            Sequence[NoSeq] = rest;
        }
        return 0;
    }
    /*void DecToBin(int Dec){
		  int i;
		  for(i=0; i<16; i++){
		     Sequence[i] = Dec >> i & 1;

		  }
		}*/

    void Draw(float Seed, int NoIter, int RozX, int RozY, int start, int koniec, float RXMin, float RXMax, float RYMin, float RYMax, int val, int s, int n, int size, omp_sched_t type)
    {
        float rx, ry, deltaX, deltaY, tmpLap = 0;
        int k, w, id_x, id_y;
        char tmp;

        for (k = 0; k < 16; k++)
            Sequence[k] = 0;

        sample = s;
        NoSeq = 0;
        Sequence[0] = 1;
        DecToBin(val);
        LapMin = -2;
        MinR = RXMin;
        MaxR = RXMax;
        MinX = RYMin;
        MaxX = RYMax;
        deltaX = (MaxR - MinR) / RozX;
        deltaY = (MaxX - MinX) / RozY;
        double z;
        omp_set_nested(1);
        omp_set_num_threads(n);
#pragma omp parallel private(id_x)
        {
            id_x = omp_get_thread_num();
            omp_set_schedule(type, size);
#pragma omp for private(w) schedule(runtime)
            for (w = 0; w < RozY; w++)
            {
#pragma omp parallel private(id_y) shared(id_x)
                {
                    id_y = omp_get_thread_num();
                    omp_set_schedule(type, size);
#pragma omp for private(k, rx, ry, tmpLap, z, tmp) schedule(runtime)

                    for (k = 0; k < RozX; k++)
                    {
                        rx = MinR + deltaX * k;
                        ry = MaxX - (start - 1) * deltaY - deltaY * w;
                        tmpLap = ValLap(Seed, NoIter, rx, ry, id_y, id_x);
                        if (tmpLap <= 0)
                        {
                            z = noColors * tmpLap / LapMin;
                            tmp = (int)(z) % noColors;
                            pixels[k][w][0] = tmp;
                            pixels[k][w][1] = tmp;
                            pixels[k][w][2] = tmp;
                        }
                        else
                        {
                            pixels[k][w][0] = 0;
                            pixels[k][w][1] = 0;
                            pixels[k][w][2] = 0;
                        }
                    }
                }
            }
        }
    }
    float ValLap(float Seed, int NoIter, float rx, float ry, int id_y, int id_x)
    {

        float x, sumlap, elem, ValLap;
        int i, poz, NoElem;
        float R;

        x = Seed;
        sumlap = 0;
        NoElem = 0;
        poz = 0;

        for (i = 1; i <= NoIter; i++)
        {
            if (Sequence[poz] == 0)
                R = ry;
            else
                R = rx;
            poz++;
            if (poz > NoSeq)
                poz = 0;
            x = fun(x, R);
            elem = (float)abs(dfun(x, R));
            if (elem > 1000)
            {
                ValLap = 10;
                break;
            }
            if (elem != 0)
            {
                sumlap = sumlap + (float)log2(elem);
                NoElem++;
            }
        }
        iterationCount[id_y][id_x] += i;
        if (NoElem > 0)
            ValLap = sumlap / NoElem;
        else
            ValLap = 0;
        return ValLap;
    }

    float fun(float x, float r)
    {
        float y = 0;
        switch (sample)
        {
        case (0):
            y = r * sin(x) + r;
            break;
        case (1):
            y = r * cos(x) + r;
            break;
        case (2):
            y = r * cos(x) * (1 - sin(x));
            break;
        }
        return y;
    }

    float dfun(float x, float r)
    {
        float y = 0;
        switch (sample)
        {
        case (0):
            y = r * cos(x);
            break;
        case (1):
            y = -r * sin(x);
            break;
        case (2):
            y = r * (1 - sin(x)) - 2 * cos(x) * cos(x);
            break;
        }
        return y;
    }
};

int main(int argc, char **argv) //size, ilość wątków, wielkość bloków, schedule type
{
    size = atoi(argv[1]);
    int n = atoi(argv[2]);
    int block_size = atoi(argv[3]);
    char *type = argv[4];

    iterationCount = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        iterationCount[i] = (int *)malloc(n * sizeof(int));
    pixels = (char ***)malloc(size * sizeof(char **));
    for (int i = 0; i < size; i++)
    {
        pixels[i] = (char **)malloc(size * sizeof(char *));
        for (int j = 0; j < size; j++)
        {
            pixels[i][j] = (char *)malloc(3 * sizeof(char));
        }
    }

    FILE *fp;
    int i, j;
    char filename[50];
    sprintf(filename, "%d_%d_%d_%s.ppm", size, n*n, block_size, type);
    char *comment = "# "; /* comment should start with # */
    const int MaxColorComponentValue = 255;
    int a;
    Lapunov lp;
    fp = fopen(filename, "wb"); /* b -  binary mode */
    /*write ASCII header to the file*/
    omp_sched_t ompScheduleType;
    if (strcmp(type, "static") == 0)
        ompScheduleType = omp_sched_static;
    else if (strcmp(type, "dynamic") == 0)
        ompScheduleType = omp_sched_dynamic;
    else
        ompScheduleType = omp_sched_guided;
    fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, size, size, MaxColorComponentValue);
    double t0 = omp_get_wtime();
    lp.Draw(5, 100, size, size, 0, size, -3, 9, -5, 2, 2477, 1, n, 10, ompScheduleType);
    double t1 = omp_get_wtime() - t0;
    printf("%d\t%d\t%d\t%s\t%f\n", size, n*n, block_size, type, t1); //size, ilość wątków, wielkość bloków, schedule type
    FILE *it;
    char filename2[50];
    sprintf(filename2, "%d_%d_%d_%s.txt", size, n*n, block_size, type);
    it = fopen(filename2, "w");
    for(int i =0;i<n;i++) {
        for(int j=0;j<n;j++){
            fprintf(it, "%d\t", iterationCount[i][j]);
        }
        fprintf(it, "\n");
    }

    for (int iY = 0; iY < size; iY++)
    {
        for (int iX = 0; iX < size; iX++)
        {

            fwrite(pixels[iY][iX], 1, 3, fp);
        }
    }
    fclose(fp);

    return 0;
}
