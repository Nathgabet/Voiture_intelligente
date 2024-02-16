#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//gcc -Wall threadLX.c -lpthread -o threadLX



void *func1(void *arg){

        int i;
        int *g = (int *)arg;

        for(i =0; i < 1000; i++){
                (*g)++;

        }

        pthread_exit(NULL);
}

/*

int *func2(void *arg){

        printf("cool \n");

        return 1;
}
*/

int main(void){

        pthread_t t1;
        int g = 0;

        printf("%d \n", g);

        pthread_create(&t1, NULL, *func1, &g);
        sleep(3);
        printf("%d \n",g);

        pthread_cancel(t1);
        return 0;
}
