#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct Matrix
{
    int length;
    float **values;
};

struct info
{
    int start;
    int end;
    int length;
    struct Matrix *a;
    struct Matrix *b;
    struct Matrix *c;
};

void make_matrix(struct Matrix *m, int l)
{
    m->length = l;
    m->values = malloc(l * sizeof(float *));
    for (int k = 0; k < l; k++)
    {
        m->values[k] = malloc(l * sizeof(float));
    }
}

void fill_matrix(struct Matrix *m, int l)
{
    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < l; j++)
            m->values[i][j] = (float)rand() / RAND_MAX * 5;
    }
}

void print_matrix(struct Matrix *m)
{
    for (int i = 0; i < m->length; i++)
    {
        for (int j = 0; j < m->length; j++)
        {
            printf("%f ", m->values[i][j]);
        }
        printf("\n");
    }
}

void *multiply_matrix(void *information)
{
    struct info *inf;
    inf = (struct info *)information;
    for (int i = inf->start; i < inf->end; ++i)
        for (int j = 0; j < inf->length; ++j)
            for (int k = 0; k < inf->length; ++k)
            {
                inf->c->values[i][j] += inf->a->values[i][k] * inf->b->values[k][j];
            }
    pthread_exit(NULL);
}

main(int argc, char **argv)
{

    struct Matrix a, b, c;
    int len = atoi(argv[2]);
    make_matrix(&a, len);
    make_matrix(&b, len);
    make_matrix(&c, len);
    float t = (float)atoi(argv[1]);
    int N = a.length;
    printf("Dlugosc macierzy: %d\n", len);
    fill_matrix(&a, len);
    fill_matrix(&b, len);
    fill_matrix(&c, len);

    pthread_t threads[(int)t];

    struct info *inf = malloc(t * sizeof(struct info));

    for (int i = 0; i < (int)t; i++)
    {
        inf[i].start = (int)(((float)i * 1.0 / t) * (float)len);
        inf[i].end = (int)((((float)i + 1) * 1.0 / t) * (float)len);
        inf[i].length = len;
        inf[i].a = &a;
        inf[i].b = &b;
        inf[i].c = &c;

    }
    int rc;
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    for (int i = 0; i < (int)t; i++)
    {
        rc = pthread_create(&threads[i], NULL, multiply_matrix, &inf[i]);
        if (rc)
        {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for (int i = 0; i < t; i++)
    {
        rc = pthread_join(threads[i], NULL);
        if (rc)
        {
            printf("Error:unable to join thread, %d\n", rc);
            exit(-1);
        }
    }
    gettimeofday(&t1, 0);
    long long elapsed = (t1.tv_sec-t0.tv_sec)*1000000LL + t1.tv_usec-t0.tv_usec;
    double elapsed2 = (double) elapsed/1000000;
    printf("Czas z %d watkami: %f\n\n", (int)t, elapsed2);

    pthread_exit(NULL);
}
