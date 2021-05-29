#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

int iXmax;
int iYmax;
int t;
const double CxMin = -2.5;
const double CxMax = 1.5;
const double CyMin = -2.0;
const double CyMax = 2.0;
const int MaxColorComponentValue = 255;
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

void *mandelbrot(void *id)
{

   double PixelWidth = (CxMax - CxMin) / iXmax;
   double PixelHeight = (CyMax - CyMin) / iYmax;
   double Zx, Zy;
   double Zx2, Zy2;
   int Iteration;
   const int IterationMax = 200;
   const double EscapeRadius = 2;
   double ER2 = EscapeRadius * EscapeRadius;
   double Cx, Cy;
   for (int iY = ((int)id * iYmax) / t; iY < (((int)id + 1) * iYmax) / t; iY++)
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
            color[iY][iX] = RGB[(int)id];
      }
   }
   pthread_exit(NULL);
}

int main(int argc, char **argv) // iXmax, iYmax, ilość wątków
{

   iXmax = atoi(argv[1]);
   iYmax = atoi(argv[1]);
   t = atoi(argv[2]);
   color = malloc(iYmax * sizeof(unsigned char **));
   for (int i = 0; i < iXmax; i++)
   {
      color[i] = malloc(iXmax * sizeof(unsigned char *));
   }

   pthread_t threads[(int)t];

   struct timeval t0, t1;
   gettimeofday(&t0, 0);
   for (int i = 0; i < (int)t; i++)
   {
      pthread_create(&threads[i], NULL, mandelbrot, (void *)i);
   }

   for (int i = 0; i < t; i++)
   {
      pthread_join(threads[i], NULL);
   }
   gettimeofday(&t1, 0);
   long long elapsed = (t1.tv_sec - t0.tv_sec) * 1000000LL + t1.tv_usec - t0.tv_usec;
   double elapsed2 = (double)elapsed / 1000000;
   printf("Wielkość obrazu: %d\t Ilosc watkow: %d \tCzas:%f\n\n", iXmax, (int)t, elapsed2);

   FILE *fp;
   char filename[50];
   sprintf(filename, "%d_%d.ppm", t, iXmax);

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
   pthread_exit(NULL);
}