/**
 * @file      chained_list.h
 * @authors   B06V10013
 * @date      08/04/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief     This header file defines a generic doubly-linked list (chained list)
 *            structure with support for both FIFO (queue) and LIFO (stack) modes,
 *            selectable via a compile-time macro.
 *
 *            The list supports operations such as:
 *              - Insertion at the head or tail
 *              - Removal from the head or tail
 *              - Element access by index or position
 *              - Swapping and deleting elements
 *
 *            It is designed to handle content of arbitrary size, configurable via
 *            `CONTENT_MAX_SIZE`, and restricts list length with `LIST_MAX_LEN`.
 *
 *            The API provides flexibility for embedded or general-purpose usage,
 *            with a focus on memory-safe operations and clear error reporting
 *            through the `err_t` enumeration.
 */

#ifndef CHIANED_LIST_CHAINED_LIST_H
#define CHIANED_LIST_CHAINED_LIST_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include <glob.h>

#define LIST_MAX_LEN            100
#define CONTENT_MAX_SIZE        512
#define LIST_NAME_SIZE          32


#define LIST_MODE               0       // 0 : Fifo (queue) mode  // 1 : Lifo (stack) mode

#if LIST_MODE
#define list_add(listHandler, content) list_insert_tail(listHandler, content)
#define list_pop(listHandler, content) list_pop_tail(listHandler, content)
#else
#define list_add(listHandler, content) list_insert_tail(listHandler, content)
#define list_pop(listHandler, content) list_pop_head(listHandler, content)
#endif


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

err_t list_insert_head(list_handler_t *listHandler, void *content);
err_t list_insert_tail(list_handler_t *listHandler, void *content);
err_t list_pop_head(list_handler_t *listHandler, void *content);
err_t list_pop_tail(list_handler_t *listHandler, void *content);
err_t list_insert_element(list_handler_t *listHandler,bool before,chained_list_t *place, void *content);

chained_list_t *list_get_first_element(list_handler_t *listHandler);
chained_list_t *list_get_last_element(list_handler_t *listHandler);
chained_list_t *list_get_element_by_index(list_handler_t *listHandler, uint16_t index);
void list_delete_element(list_handler_t *listHandler,chained_list_t *element);
err_t list_swap_element(list_handler_t *listHandler, chained_list_t *b, chained_list_t *c);

#endif //CHIANED_LIST_CHAINED_LIST_H
