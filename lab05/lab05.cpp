#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

int size;
char * filename;
int **Maze;
int start[2] = {1, 1};
int id = 1;
pthread_mutex_t **mutex_maze;
pthread_mutex_t mutex_id;
int vectors[4][2] = {{-1, 0}, {1, 0}, {0, 1}, {0, -1}}; //dół, góra, prawo, lewo
unsigned char black[3] = {0,0,0};
unsigned char RGB[20][3] = {
{28, 57, 44},{246, 93, 176},{196, 35, 204},{160, 2, 23},{62, 224, 225},{102, 199, 15},{112, 32, 37},{238, 169, 255},{107, 43, 101},{82, 144, 152},
{158, 199, 49},{75, 136, 76},{93, 140, 245},{41, 80, 108},{74, 219, 234},{48, 225, 191},{234, 128, 147},{26, 168, 211},{81, 129, 190},{1, 69, 44}};
void printMaze()
{
    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
            printf("%02d", Maze[x][y]);
        printf("\n");
    }
    printf("\n");
}

void read_from_file(){
    FILE *myFile;
    myFile = fopen(filename, "r");
    unsigned char temp;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size + 1; j++)
        {
            fscanf(myFile, "%c", &temp);
            if(temp=='#')
                Maze[i][j] = -1;
            else if(temp==' ')
                Maze[i][j] = 0;            
        }
        
    }
    printMaze();
}

void *maze(void *start)
{ //start[0] to x
    pthread_t* threads[50];

    int *position = (int *)start;
    int x = position[0];
    int y = position[1];
    pthread_mutex_lock(&mutex_id);
    int local_id = id++;
    pthread_mutex_unlock(&mutex_id);
    printf("%d %d %d\n", x, y, local_id);

    int thread_count = 0;
    int path_count = 0;
    int possible_paths[4][2] = {};
    bool is_mutexed = false;
    while (true)
    {
        path_count = 0;
        if (!is_mutexed)
            pthread_mutex_lock(&mutex_maze[x][y]);
        if (Maze[x][y] != 0)
        {
            pthread_mutex_unlock(&mutex_maze[x][y]);
            break;
        }
        Maze[x][y] = local_id;
        pthread_mutex_unlock(&mutex_maze[x][y]);

        
        for (int i = 0; i < 4; i++)
        {
            pthread_mutex_lock(&mutex_maze[x + vectors[i][0]][y + vectors[i][1]]);
            if (Maze[x + vectors[i][0]][y + vectors[i][1]] == 0) {
                possible_paths[path_count][0] = vectors[i][0];
                possible_paths[path_count][1] = vectors[i][1];
                path_count++;
            }
            else
            {
                pthread_mutex_unlock(&mutex_maze[x + vectors[i][0]][y + vectors[i][1]]);
            }
        }

        if (path_count == 0)
        {
            break;
        }
        
        if (path_count > 1)
        {
            for (int j = 1; j < path_count; j++)
            {                     //local_id +j
                threads[thread_count++] = new pthread_t;// = thread;
                int *new_position = new int[2];
                new_position[0] = x + possible_paths[j][0];
                new_position[1] = y + possible_paths[j][1];
                pthread_create(threads[thread_count - 1], NULL, maze, (void *)new_position);
                pthread_mutex_unlock(&mutex_maze[new_position[0]][new_position[1]]);
            }
        }

        x += possible_paths[0][0];
        y += possible_paths[0][1];
        is_mutexed = true;
        //pthread_mutex_unlock(&mutex_maze[x][y]);
    }
    for (int i = 0; i < thread_count; i++)
        pthread_join(*threads[i], NULL);
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    filename = argv[1];
    size = atoi(argv[2]);
    mutex_maze = (pthread_mutex_t **)malloc(size * sizeof(pthread_mutex_t *));
    Maze = (int**)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
    {
        mutex_maze[i] = (pthread_mutex_t *)malloc(size * sizeof(pthread_mutex_t));
        Maze[i] = (int *)malloc(size * sizeof(int));

    }
    read_from_file();

    for (int i = 0; i < size; i++)
    {

        for (int j = 0; j < size; j++)
        {
            pthread_mutex_init(&mutex_maze[i][j], NULL);
        }
    }
    pthread_mutex_init(&mutex_id, NULL);

    pthread_t first_thread;
    printMaze();
    pthread_create(&first_thread, NULL, maze, (void *)&start);
    pthread_join(first_thread, NULL);
    printMaze();

    FILE *fp;
   char filename[50];
   sprintf(filename, "%d.ppm", size);

   fp = fopen(filename, "wb"); /* b -  binary mode */
   fprintf(fp, "P6\n # \n %d\n %d\n %d\n", size, size, 255);
   for (int iY = 0; iY < size; iY++)
   {
      for (int iX = 0; iX < size; iX++)
      {
          if(Maze[iY][iX] == -1)
            fwrite(black, 1, 3, fp);
        else
         fwrite(RGB[(Maze[iY][iX])%20], 1, 3, fp);
      }
   }
   fclose(fp);


}
