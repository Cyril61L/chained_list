/**
 * @file      test_sort.c
 * @authors   B06V10013
 * @date      25/03/2026
 * @copyright 2026 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief 
 */

#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>

#include "chained_list.h"

#define LIST_LENGTH 10000
#define LIST_ITER   10
#define RANDOM      100000001

list_handler_t test_sort;

int main(void) {
    uint64_t max=0, min=0xffffffffffffffff, moyenne=0;

    srand( time( NULL ) );

    for (uint32_t i = 0; i < LIST_ITER; i++)
    {
        list_init(&test_sort,1,LIST_MODE_FIFO);

        uint8_t list_1[LIST_LENGTH] = {2,8,4,9,1};
        uint8_t list_2[LIST_LENGTH] = {9,8,4,7,1};
        uint8_t list_3[LIST_LENGTH] = {2,8,4,7,9};
        uint8_t list_4[LIST_LENGTH] = {1,2,3,4,5};

        for (uint32_t j = 0; j < LIST_LENGTH; j++)
        {
            uint8_t content = j;
            list_add_id(&test_sort,&content,rand()%RANDOM);
        }

        //list_print(&test_sort);
        struct timespec ts_1,ts_2;
        clock_gettime(CLOCK_MONOTONIC, &ts_1);
        //list_sort_by_id(&test_sort);
        //sort(&test_sort);
        //sort2(&test_sort);
        insertion_sort(&test_sort);
        clock_gettime(CLOCK_MONOTONIC, &ts_2);
        signed long int delta_us =
            (ts_1.tv_sec == ts_2.tv_sec) ? (ts_2.tv_nsec - ts_1.tv_nsec) / 1000 : ((1000000000 - ts_1.tv_nsec) + ts_2.tv_nsec) / 1000;
        //printf("Delta %lu µs\n", delta_us);
        //list_print(&test_sort);
        if (delta_us < min)
        {
            min = delta_us;
        }
        if (delta_us > max)
        {
            max = delta_us;
        }

        moyenne += delta_us;
        list_free(&test_sort);
    }

    moyenne = moyenne / LIST_ITER;
    printf("Moy=%lu us min=%lu us max=%lu us\n",moyenne,min,max);
    return 0;
}