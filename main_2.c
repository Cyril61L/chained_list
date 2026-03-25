/**
 * @file      main_2.c
 * @authors   B06V10013
 * @date      17/09/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief 
 */

#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>

#include "chained_list.h"

typedef void (*func_ptr_t)(char *);

void func(char *args)
{
    printf("func %s\n",args);
}

int add(int a, int b)
{
    printf("%d + %d",a,b);
    return a + b;
}

int sub(int a, int b)
{
    printf("%d - %d",a,b);
    return a - b;
}

int mul(int a, int b)
{
    printf("%d x %d",a,b);
    return a * b;
}

typedef struct
{
    char name[32];
    bool state;
    int (*func)(int,int);
}seq_t;

list_handler_t lifo_seq;

int main(void) {
    srand( time( NULL ) );

    seq_t seq1 = {
        .name = "task1",
        .state = false,
        .func = add,
    };

    seq_t seq2 = {
        .name = "task2",
        .state = false,
        .func = sub,
    };

    seq_t seq3 = {
        .name = "task3",
        .state = false,
        .func = mul,
    };

    list_init(&lifo_seq,sizeof(seq_t),LIST_MODE_CIRCULAR);

    list_add(&lifo_seq,&seq1);
    list_add(&lifo_seq,&seq2);
    list_add(&lifo_seq,&seq3);
    list_add(&lifo_seq,&seq1);
    list_add(&lifo_seq,&seq2);
    list_add(&lifo_seq,&seq3);

    seq_t tmp;
    int result = 8;
    for (uint8_t i=0; i<15; i++)
    {
        if (list_browse(&lifo_seq,NEXT_ELEM,&tmp) == ERR_OK)
        {
            result = tmp.func(result, rand()%10);
            printf(" = %d\n",result);
        } else
        {
            printf("End result %d\n",result);
            break;
        }
    }
    list_free(&lifo_seq);
    return 0;
}