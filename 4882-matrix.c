#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct
{
    int id;
    int *DATAROW;
    int *DATACOL;
    int count;
    pthread_t thread;

} thread_pac;

thread_pac *threads;

int **DATARES;
int countAx = 0;
int countAy = 0;
int countBx = 0;
int countBy = 0;
int diff = 0;

void fileWrite(int output, int flag,double time)
{
    FILE *pFile;
    if (flag == -1)
        pFile = fopen("output.txt", "w");
    else
        pFile = fopen("output.txt", "a");
    if (flag == 1)
    {
        fprintf(pFile, "\n");
    }
    else if (flag <= 0)
    {
        fprintf(pFile, " [ %d ]", output);
    } else if (flag == 2){
        fprintf(pFile, " The program took [ %lf ] seconds to execute ", time);
    }

    fclose(pFile);
}

void *multiplyRow(void *arg)
{
    int *data = (int *)arg;

    int i, j, col, row;
    for (j = 0; j < countBy; j++)
    {
        int *ptr = threads[data[0]].DATACOL;
        for (i = 0; i < countAy; i++)
        {
            int row = threads[data[0]].DATAROW[i];
            int col = ptr[j];
            DATARES[data[0]][j] += row * col;
            //if (i != countBy)
            ptr = ptr += diff;
        }
    }

    pthread_exit(NULL);
}

void *multiplyElem(void *arg)
{

    int *data = (int *)arg;
    int i, j, col, row;
    int ColNo = data[0] % countBy;
    int *ptr = threads[data[0]].DATACOL;
    for (j = 0; j < countBx; j++)
    {
        int row = threads[data[0]].DATAROW[j];
        int col = ptr[ColNo];
        DATARES[threads[data[0]].count][ColNo] += row * col;
        //if (j != countBy)
        ptr = ptr += diff;
    }
    pthread_exit(NULL);
}

int main()
{

    int i, j, k;
    FILE *pFile;

    int **martixA;
    int **martixB;
    int element;

    pFile = fopen("input.txt", "r");
    if (pFile != NULL)
    {
        fscanf(pFile, "%d", &countAx);
        fscanf(pFile, "%d", &countAy);
        martixA = (int **)malloc(countAx * sizeof(int *));
        for (i = 0; i < countAx; i++)
            martixA[i] = (int *)malloc(countAy * sizeof(int));
        for (i = 0; i < countAx; i++)
        {
            for (j = 0; j < countAy; j++)
            {
                fscanf(pFile, "%d", &element);
                martixA[i][j] = element;
            }
        }

        fscanf(pFile, "%d", &countBx);
        fscanf(pFile, "%d", &countBy);

        martixB = (int **)malloc(countBx * sizeof(int *));
        for (i = 0; i < countBx; i++)
            martixB[i] = (int *)malloc(countBy * sizeof(int));
        for (i = 0; i < countBx; i++)
        {
            for (j = 0; j < countBy; j++)
            {
                fscanf(pFile, "%d", &element);
                martixB[i][j] = element;
            }
        }

        fclose(pFile);
    }else{
        printf("file Not Found\n");
        exit(-1);
    }

    if (countAy != countBx||countAx==0||countAy==0||countBx==0||countBy==0)
    {
        printf("INVALID OPERATION INDEX MATRIX\n");
        return 0;
    }

    diff = &martixB[1][0] - &martixB[0][0];
    //printf(" dfiff = %d\n", diff);

    //Print Index of Output array
    fileWrite(countAx, -1,0);
    fileWrite(countBy, 0,0);
    


    DATARES = (int **)malloc(countAx * sizeof(int *));
    for (i = 0; i < countAx; i++)
        DATARES[i] = (int *)malloc(countBy * sizeof(int));

    for (i = 0; i < countAx; i++)
        for (j = 0; j < countBy; j++)
            DATARES[i][j] = 0;

    threads = (thread_pac *)malloc(countAx * sizeof(thread_pac));

    clock_t t;
    t = clock();

    for (i = 0; i < countAx; i++)
    {
        threads[i].DATAROW = martixA[i];
        threads[i].DATACOL = martixB[0];
        threads[i].id = i;
        pthread_create(&threads[i].thread, NULL, multiplyRow, (void *)(&threads[i].id));
    }


    for (i = 0; i < countAx; i++)
    {
        pthread_join(threads[i].thread, NULL);
    }

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC;
    printf("******Matrix MUL-1******\n");
    printf("The program took %f seconds to execute\n", time_taken);

    fileWrite(0, 1,0);

    for (i = 0; i < countAx; i++)
    {
        for (j = 0; j < countBy; j++)
        {
            fileWrite(DATARES[i][j], 0,0);
            printf(" %d ", DATARES[i][j]);
        }
        fileWrite(0, 1,0);
        printf("\n");
    }
    fileWrite(0, 1,0); // one Line Down
    fileWrite(0, 2,time_taken);

    free(threads);
    free(DATARES);

    fileWrite(0, 1,0);

    //==========================ELEMENT BY ELEMENT ======================================================

    fileWrite(0, 1,0);
    fileWrite(countAx, 0,0);
    fileWrite(countBy, 0,0);



    DATARES = (int **)malloc(countAx * sizeof(int *));
    for (i = 0; i < countAx; i++)
        DATARES[i] = (int *)malloc(countBy * sizeof(int));

    for (i = 0; i < countAx; i++)
        for (j = 0; j < countBy; j++)
            DATARES[i][j] = 0;

    threads = (thread_pac *)malloc(countAx * countBy * sizeof(thread_pac));
    int ROWNO = 0;

    clock_t t2;
    t2 = clock();
    for (i = 0; i < countAx * countBy; i++)
    {
        if (i % countBy == 0 && i != 0)
            ROWNO++;

        threads[i].DATAROW = martixA[ROWNO];
        threads[i].DATACOL = martixB[0];
        threads[i].id = i;
        threads[i].count = ROWNO;
        pthread_create(&threads[i].thread, NULL, multiplyElem, (void *)(&threads[i].id));
    }


    for (i = 0; i < countAx * countBy; i++)
    {
        pthread_join(threads[i].thread,NULL);
    }
    t2 = clock() - t2;
    double time_taken2 = ((double)t2) / CLOCKS_PER_SEC;
    printf("******Matrix MUL-2******\n");
    printf("The program took %f seconds to execute\n", time_taken2);

    for (i = 0; i < countAx; i++)
    {
        fileWrite(0, 1,0); // one Line Down
        for (j = 0; j < countBy; j++)
        {
            fileWrite(DATARES[i][j], 0,0);
            printf(" %d ", DATARES[i][j]);
        }
        printf("\n");
    }
    
    fileWrite(0, 1,0); // one Line Down
    fileWrite(0, 2,time_taken2);
    free(threads);
    free(DATARES);

    return 0;
}
