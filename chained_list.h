/**
 * @file      chained_list.h
 * @authors   B06V10013
 * @date      08/04/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief 
 */
#ifndef CHIANED_LIST_CHAINED_LIST_H
#define CHIANED_LIST_CHAINED_LIST_H

#include <stdint-gcc.h>
#include <stdbool.h>

typedef struct elem {
    uint8_t x;
    struct elem *prev;
    struct elem *next;
}chained_list_t;

void init_list(chained_list_t *list);
chained_list_t *get_first_element(chained_list_t *list);
chained_list_t *get_last_element(chained_list_t *list);
chained_list_t *add(chained_list_t **list, uint8_t value);
void pop(chained_list_t *list, uint8_t *value);
void show_list(chained_list_t *list);
        uint8_t get_size_list(chained_list_t *list);
        chained_list_t *insert_element(chained_list_t *list,bool before,chained_list_t *place, uint8_t value);
        void *delete_element(chained_list_t *list,chained_list_t *element);
void swap_element(chained_list_t *b, chained_list_t *c);
void sort(chained_list_t *list);

#endif //CHIANED_LIST_CHAINED_LIST_H
