#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define max(x, y) ((x) > (y) ? (x) : (y))
int t;
int size;
int ulam_get_map(int x, int y, int n){
    x -= (n - 1) / 2;
    y -= n / 2;
    int mx = abs(x),
        my = abs(y);
    int l = 2 * max(mx, my);
    int d = y >= x ? l * 3 + x + y : l - x - y;
    return pow(l - 1, 2) + d;
}
int isprime(int n){
    for (int p = 2; p * p <= n; p++)
        if (n % p == 0)
            return 0;
    return n > 2;
}
unsigned char ***color;
unsigned char RGB[100][3] = {{255, 255, 255},  {152, 134, 191},  {109, 86, 129},  {1, 0, 136},  {189, 133, 185},  {144, 19, 20},  {41, 35, 96},  {42, 246, 155},  {126, 171, 242},  {234, 102, 125},  {162, 122, 202},  {31, 13, 55},  {20, 231, 113},  {225, 218, 217},  {57, 240, 73},  {133, 87, 224},  {140, 39, 24},  {80, 192, 188},  {26, 31, 159},  {142, 130, 181},  {128, 228, 74},  {91, 35, 132},  {234, 118, 215},  {163, 168, 142},  {118, 115, 114},  {184, 136, 216},  {56, 103, 230},  {139, 186, 116},  {187, 170, 105},  {43, 33, 40},  {7, 221, 121},  {70, 0, 109},  {253, 167, 224},  {90, 70, 131},  {109, 248, 177},  {49, 249, 8},  {60, 216, 115},  {59, 128, 34},  {212, 169, 47},  {13, 61, 40},  {37, 4, 76},  {227, 41, 137},  {170, 12, 94},  {143, 117, 203},  {246, 30, 11},  {219, 30, 96},  {77, 117, 244},  {100, 25, 30},  {181, 107, 183},  {102, 22, 175},  {40, 191, 192},  {61, 55, 211},  {61, 140, 64},  {145, 13, 97},  {224, 72, 74},  {96, 19, 92},  {145, 116, 108},  {78, 179, 213},  {133, 225, 162},  {176, 190, 206},  {132, 97, 203},  {107, 184, 234},  {86, 219, 201},  {6, 132, 116},  {213, 24, 134},  {182, 106, 48},  {196, 15, 173},  {150, 79, 49},  {201, 81, 76},  {177, 119, 199},  {196, 150, 23},  {37, 186, 132},  {101, 113, 237},  {161, 227, 161},  {209, 144, 149},  {29, 116, 117},  {190, 73, 104},  {137, 149, 20},  {154, 170, 175},  {154, 236, 218},  {207, 191, 93},  {113, 25, 130},  {28, 251, 17},  {13, 241, 71},  {214, 214, 197},  {23, 61, 246},  {232, 147, 149},  {150, 230, 250},  {210, 208, 203},  {182, 208, 93},  {157, 26, 157},  {53, 232, 132},  {54, 199, 91},  {4, 56, 212},  {205, 116, 131},  {88, 242, 37},  {29, 123, 233},  {38, 24, 61},  {128, 85, 131},  {168, 17, 167}};
void *spiral(void *id){
    int nr = (int)id;
    int n = sqrt(t);
    int r = ((nr) / n) +1;
    int c = nr % n;
    if(c == 0)
        c = n;
    int len = size / n;
    for (int x = (r - 1) * len; x < r * len; x++)
        for (int y = (c - 1) * len; y < c * len; y++)
        {
            if (isprime(ulam_get_map(x, y, size)))
                color[x][y] = RGB[(int)id+1];
            else
                color[x][y] = RGB[0];
        }
    pthread_exit(NULL);
}
int main(int argc, char **argv) // n^2 - ilość wątków, wielkość - wielokrotność 420
{
    t = atoi(argv[1]);
    size = atoi(argv[2]);
    color = malloc(size * sizeof(unsigned char **));
    for (int i = 0; i < size; i++)
        color[i] = malloc(size * sizeof(unsigned char *));
    pthread_t threads[(int)t];
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    for (int i = 0; i < (int)t; i++)
        pthread_create(&threads[i], NULL, spiral, (void *)i);
    for (int i = 0; i < t; i++)
        pthread_join(threads[i], NULL);
    gettimeofday(&t1, 0);
    long long elapsed = (t1.tv_sec - t0.tv_sec) * 1000000LL + t1.tv_usec - t0.tv_usec;
    double elapsed2 = (double)elapsed / 1000000;
    printf("Wielkość obrazu: %d\t Ilosc watkow: %d \tCzas:%f\n\n", size, (int)t, elapsed2);
    FILE *fp;
    char filename[50];
    sprintf(filename, "%d_%d.ppm", t, size);
    fp = fopen(filename, "wb"); /* b -  binary mode */
    fprintf(fp, "P6\n # \n %d\n %d\n %d\n", size, size, 255);
    for (int iY = 0; iY < size; iY++)
        for (int iX = 0; iX < size; iX++)
            fwrite(color[iY][iX], 1, 3, fp);
    fclose(fp);
    pthread_exit(NULL);
}