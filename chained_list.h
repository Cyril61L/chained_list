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
#include <glob.h>

#define LIST_MAX_LEN            100
#define CONTENT_MAX_SIZE        512
#define LIST_NAME_SIZE          32

typedef enum {
    ERR_OK = 0,
    ERR_MALLOC_FAILED = 1,
    ERR_PARAM = 2,
    ERR_NULL_POINTER = 3,
    ERR_LIST_FULL = 4,
}err_t;


typedef struct elem {
    void *content;
    struct elem *prev;
    struct elem *next;
}chained_list_t;

typedef struct {
    char *name[32];
    size_t contentLen;
    uint8_t listLength;
    chained_list_t *head;
    chained_list_t *tail;
}list_handler_t;

err_t list_init(list_handler_t *listHandler, const char *name, size_t len);
uint16_t list_get_size(list_handler_t *listHandler);

err_t list_add(list_handler_t *listHandler, void *content);
err_t list_pop(list_handler_t *listHandler, void *content);

err_t list_insert_element(list_handler_t *listHandler,bool before,chained_list_t *place, void *content);
chained_list_t *list_get_first_element(list_handler_t *listHandler);
chained_list_t *list_get_last_element(list_handler_t *listHandler);
void list_delete_element(list_handler_t *listHandler,chained_list_t *element);
err_t list_swap_element(chained_list_t *b, chained_list_t *c);

#endif //CHIANED_LIST_CHAINED_LIST_H
