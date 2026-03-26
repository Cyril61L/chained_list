/**
 * @file      chained_list.h
 * @authors   B06V10013
 * @date      08/04/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief     This header file defines a generic doubly-linked list (chained list)
 *            structure with support for both FIFO (queue) and LIFO (stack) modes.
 *
 *            The list supports operations such as:
 *              - Insertion at the tail
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

#ifndef CHAINED_LIST_H
#define CHAINED_LIST_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include <glob.h>
#include <pthread.h>

#define LIST_MAX_LEN     10000
#define CONTENT_MAX_SIZE 512

typedef enum { LIST_MODE_FIFO, LIST_MODE_LIFO , LIST_MODE_CIRCULAR} list_mode_e;

typedef enum {
    ERR_OK = 0,
    ERR_MALLOC_FAILED = 1,
    ERR_PARAM = 2,
    ERR_NULL_POINTER = 3,
    ERR_LIST_FULL = 4,
    ERR_NOT_FOUND = 5,
} err_t;

typedef enum {
    FIRST_ELEM = 0,
    LAST_ELEM = 1,
    CURRENT_ELEM = 2,
    NEXT_ELEM = 3,
    PREV_ELEM = 4,
}list_move_e;

typedef struct elem {
    void* content;
    uint64_t id;
    struct elem* prev;
    struct elem* next;
} chained_list_t;

typedef struct list_handler {
    size_t contentLen;
    uint32_t listLength;
    list_mode_e listMode;
    chained_list_t* head;
    chained_list_t* current;
    chained_list_t* tail;
    pthread_mutex_t lock;
} list_handler_t;

err_t list_init(list_handler_t* listHandler, size_t len, list_mode_e mode);
err_t list_free(list_handler_t* listHandler);
uint16_t list_get_size(list_handler_t* listHandler);
err_t list_print(list_handler_t* listHandler);

err_t list_add(list_handler_t* listHandler, const void* content);
err_t list_add_id(list_handler_t* listHandler, const void* content, uint64_t id);
err_t list_pop(list_handler_t* listHandler, void* content);

err_t list_browse(list_handler_t* listHandler, list_move_e move, void* content);
err_t list_sort_by_id(list_handler_t* listHandler);
void sort2(list_handler_t *list);
void sort(list_handler_t *list);
void insertion_sort(list_handler_t *list);
err_t list_get_element_by_index(list_handler_t* listHandler, uint16_t index, void* content, bool delete);
err_t list_get_element_by_id(list_handler_t* listHandler, uint64_t id, void* content, bool delete);

#endif //CHAINED_LIST_H
