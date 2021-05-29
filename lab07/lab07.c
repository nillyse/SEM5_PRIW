#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define max(x, y) ((x) > (y) ? (x) : (y))
int size;
int **iterationCount;
int ulam_get_map(int x, int y, int n)
{
    x -= (n - 1) / 2;
    y -= n / 2;
    int mx = abs(x),
        my = abs(y);
    int l = 2 * max(mx, my);
    int d = y >= x ? l * 3 + x + y : l - x - y;
    return pow(l - 1, 2) + d;
}
int isprime(int n, int id_x, int id_y)
{
    for (int p = 2; p * p <= n; p++)
    {
        iterationCount[id_x][id_y]++;
        if (n % p == 0)
            return 0;
    }
    return n > 2;
}
unsigned char ***color;
unsigned char RGB[100][3] = {{175, 190, 23}, {152, 134, 191}, {109, 86, 129}, {1, 0, 136}, {189, 133, 185}, {144, 19, 20}, {41, 35, 96}, {42, 246, 155}, {126, 171, 242}, {234, 102, 125}, {162, 122, 202}, {31, 13, 55}, {20, 231, 113}, {225, 218, 217}, {57, 240, 73}, {133, 87, 224}, {140, 39, 24}, {80, 192, 188}, {26, 31, 159}, {142, 130, 181}, {128, 228, 74}, {91, 35, 132}, {234, 118, 215}, {163, 168, 142}, {118, 115, 114}, {184, 136, 216}, {56, 103, 230}, {139, 186, 116}, {187, 170, 105}, {43, 33, 40}, {7, 221, 121}, {70, 0, 109}, {253, 167, 224}, {90, 70, 131}, {109, 248, 177}, {49, 249, 8}, {60, 216, 115}, {59, 128, 34}, {212, 169, 47}, {13, 61, 40}, {37, 4, 76}, {227, 41, 137}, {170, 12, 94}, {143, 117, 203}, {246, 30, 11}, {219, 30, 96}, {77, 117, 244}, {100, 25, 30}, {181, 107, 183}, {102, 22, 175}, {40, 191, 192}, {61, 55, 211}, {61, 140, 64}, {145, 13, 97}, {224, 72, 74}, {96, 19, 92}, {145, 116, 108}, {78, 179, 213}, {133, 225, 162}, {176, 190, 206}, {132, 97, 203}, {107, 184, 234}, {86, 219, 201}, {6, 132, 116}, {213, 24, 134}, {182, 106, 48}, {196, 15, 173}, {150, 79, 49}, {201, 81, 76}, {177, 119, 199}, {196, 150, 23}, {37, 186, 132}, {101, 113, 237}, {161, 227, 161}, {209, 144, 149}, {29, 116, 117}, {190, 73, 104}, {137, 149, 20}, {154, 170, 175}, {154, 236, 218}, {207, 191, 93}, {113, 25, 130}, {28, 251, 17}, {13, 241, 71}, {214, 214, 197}, {23, 61, 246}, {232, 147, 149}, {150, 230, 250}, {210, 208, 203}, {182, 208, 93}, {157, 26, 157}, {53, 232, 132}, {54, 199, 91}, {4, 56, 212}, {205, 116, 131}, {88, 242, 37}, {29, 123, 233}, {38, 24, 61}, {128, 85, 131}, {168, 17, 167}};
unsigned char white[3] = {255, 255, 255};
void spiral(int n)
{
    int id_x, id_y;
    int x, y;
    omp_set_nested(1);
    omp_set_num_threads(n);
#pragma omp parallel private(id_x)
    {
        id_x = omp_get_thread_num();
#pragma omp for private(x)
        for (x = 0; x < size; x++)
        {
#pragma omp parallel private(id_y) shared(id_x, x)
            {
                id_y = omp_get_thread_num();
#pragma omp for private(y)
                for (y = 0; y < size; y++)
                {

                    if (isprime(ulam_get_map(x, y, size), id_x, id_y))
                        color[x][y] = white;
                    else
                        color[x][y] = RGB[(n * id_x + id_y) % 100];
                }
            }
        }
    }
}
int main(int argc, char **argv) // n^2 - ilość wątków, wielkość - wielokrotność 420
{

    int n = atoi(argv[1]);
    size = atoi(argv[2]);
    color = malloc(size * sizeof(unsigned char **));
    for (int i = 0; i < size; i++)
        color[i] = malloc(size * sizeof(unsigned char *));
    iterationCount = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        iterationCount[i] = malloc(n * sizeof(int));
    
    float t0 = omp_get_wtime();
    spiral(n);
    float t1 = omp_get_wtime() - t0;
    printf("%d\t%d\t%f\n\n", size, n*n, t1); //wielkość obrazu, ilość wątków, czas

    FILE *fp;
    char filename[50];

    FILE *it;
    char filename2[50];
    sprintf(filename, "%d_%d.ppm", n*n, size);
    fp = fopen(filename, "wb"); /* b -  binary mode */
    fprintf(fp, "P6\n # \n %d\n %d\n %d\n", size, size, 255);

    sprintf(filename2, "%d_%d.txt", n*n, size);
    it = fopen(filename2, "w");
    for(int i =0;i<n;i++) {
        for(int j=0;j<n;j++){
            fprintf(it, "%d\t", iterationCount[i][j]);
        }
        fprintf(it, "\n");
    }

    for (int iY = 0; iY < size; iY++)
        for (int iX = 0; iX < size; iX++)
            fwrite(color[iY][iX], 1, 3, fp);
    fclose(fp);
}