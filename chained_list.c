/**
 * @file      chained_list.c
 * @authors   B06V10013
 * @date      08/04/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief
 */
#include "chained_list.h"

#include <memory.h>
#include <stdbool.h>
#include <malloc.h>
#include <pthread.h>

#define ID_UNDEFINED 0xFFFFFFFFFFFFFFFF

chained_list_t* list_get_first_element(list_handler_t listHandler);
chained_list_t* list_get_last_element(list_handler_t listHandler);
err_t list_insert_element(list_handler_t listHandler, bool before, chained_list_t* place, const void* content, const uint64_t* id);
void list_delete_element(list_handler_t listHandler, chained_list_t* element);
err_t list_swap_element(list_handler_t listHandler, chained_list_t* a, chained_list_t* b);
chained_list_t* list_get_element_by_id_(list_handler_t listHandler, uint64_t id);
chained_list_t* list_get_element_by_index_(list_handler_t listHandler, uint16_t index);

/**
 * @brief Initialize a new linked list handler.
 *
 * @param listHandler Pointer to the list handler to initialize.
 * @param name        Name of the list (copied into listHandler->name).
 * @param len         Size in bytes of each element’s content.
 * @param mode        List operating mode (FIFO or LIFO).
 * @return            ERR_OK if success, ERR_PARAM if invalid parameters.
 */
err_t list_init(list_handler_t* listHandler, const char* name, size_t len, list_mode_t mode) {
    if (len > CONTENT_MAX_SIZE || len <= 0) {
        return ERR_PARAM;
    }
    *listHandler = malloc(sizeof(struct list_handler));
    if (*listHandler) {
        strncpy((char*)(*listHandler)->name, name, LIST_NAME_SIZE);
        (*listHandler)->head = NULL;
        (*listHandler)->tail = NULL;
        (*listHandler)->listMode = mode;
        (*listHandler)->contentLen = len;
        (*listHandler)->listLength = 0;
    }
    pthread_mutex_init(&(*listHandler)->lock, NULL);
    return ERR_OK;
}

/**
 * @brief Free all memory allocated for the list and its elements.
 *
 * @param listHandler Pointer to the list handler to free.
 * @return            ERR_OK if success, ERR_NULL_POINTER if pointer is invalid.
 */
err_t list_free(list_handler_t listHandler) {

    if (listHandler != NULL) {
        chained_list_t* elem;
        pthread_mutex_lock(&listHandler->lock);
        do {
            elem = list_get_last_element(listHandler);
            list_delete_element(listHandler, elem);
        } while (elem != NULL);
        pthread_mutex_unlock(&listHandler->lock);
        pthread_mutex_destroy(&listHandler->lock);
        free(listHandler);
        return ERR_OK;
    }
    return ERR_NULL_POINTER;
}

/**
 * @brief Get the first element of the list.
 *
 * @param listHandler List handler.
 * @return            Pointer to the first element, NULL if list is empty.
 */
chained_list_t* list_get_first_element(list_handler_t listHandler) {
    if (listHandler->tail != NULL) {
        chained_list_t* p = listHandler->tail;
        while (p->prev != NULL) {
            p = p->prev;
        }
        return p;
    }
    return NULL;
}

/**
 * @brief Get the last element of the list.
 *
 * @param listHandler List handler.
 * @return            Pointer to the last element, NULL if list is empty.
 */
chained_list_t* list_get_last_element(list_handler_t listHandler) {
    if (listHandler->head != NULL) {
        chained_list_t* p = listHandler->head;
        while (p->next != NULL) {
            p = p->next;
        }
        return p;
    }
    return NULL;
}

/**
 * @brief Add an element at the end of the list.
 *
 * @param listHandler List handler.
 * @param content     Pointer to the data to insert (copied internally).
 * @return            ERR_OK if success,
 *                    ERR_PARAM if invalid arguments,
 *                    ERR_LIST_FULL if list is full,
 *                    ERR_MALLOC_FAILED if memory allocation fails.
 */
err_t list_add(list_handler_t listHandler, const void* content) {
    return list_add_id(listHandler, content, NULL);
}

/**
 * @brief Add an element at the end of the list.
 *
 * @param listHandler List handler.
 * @param content     Pointer to the data to insert (copied internally).
 * @param id          element id, NULL if not use
 * @return            ERR_OK if success,
 *                    ERR_PARAM if invalid arguments,
 *                    ERR_LIST_FULL if list is full,
 *                    ERR_MALLOC_FAILED if memory allocation fails.
 */
err_t list_add_id(list_handler_t listHandler, const void* content, const uint64_t* id) {
    if (listHandler == NULL || content == NULL) {
        return ERR_PARAM;
    }
    pthread_mutex_lock(&listHandler->lock);
    if (listHandler->listLength == LIST_MAX_LEN) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_LIST_FULL;
    }

    if (id != NULL && (list_get_element_by_id_(listHandler, *id) != NULL || *id == ID_UNDEFINED)) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_PARAM;
    }

    // Allocate new node and content
    chained_list_t* newElement = malloc(sizeof(chained_list_t));
    if (newElement == NULL) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_MALLOC_FAILED;
    }

    void* p = malloc(listHandler->contentLen);
    if (p == NULL) {
        free(newElement);
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_MALLOC_FAILED;
    }
    memcpy(p, content, listHandler->contentLen);
    newElement->content = p;
    newElement->next = NULL;
    if (id) {
        newElement->id = *id;
    } else {
        newElement->id = ID_UNDEFINED;
    }
    newElement->prev = listHandler->tail;

    if (listHandler->tail) {
        listHandler->tail->next = newElement;
    } else {
        // List was empty
        listHandler->head = newElement;
    }

    listHandler->tail = newElement;
    listHandler->listLength++;
    pthread_mutex_unlock(&listHandler->lock);
    return ERR_OK;
}

/**
 * @brief Remove an element from the list (FIFO or LIFO depending on mode)
 *        and copy its content.
 *
 * @param listHandler List handler.
 * @param content     Output buffer to store the removed data.
 * @return            ERR_OK if success,
 *                    ERR_PARAM if invalid arguments or empty list,
 *                    ERR_NULL_POINTER if no element is found.
 */
err_t list_pop(list_handler_t listHandler, void* content) {
    chained_list_t* p;

    if (listHandler == NULL || content == NULL || listHandler->tail == NULL) {
        return ERR_PARAM;
    }
    // Lock
    pthread_mutex_lock(&listHandler->lock);

    if (listHandler->listMode == LIST_MODE_FIFO) {
        p = listHandler->head;
    } else {
        p = listHandler->tail;
    }
    if (p != NULL) {
        if (p->content != NULL) {
            memcpy(content, p->content, listHandler->contentLen);
            free(p->content);
        }
        if (listHandler->listMode == LIST_MODE_FIFO && p->next != NULL) {
            p->next->prev = NULL;
            listHandler->head = p->next;
        } else if (listHandler->listMode == LIST_MODE_LIFO && p->prev != NULL) {
            p->prev->next = NULL;
            listHandler->tail = p->prev;
        } else {
            // On supprimait le seul élément
            listHandler->tail = NULL;
            listHandler->head = NULL;
        }
        free(p);
        listHandler->listLength--;
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_OK;
    }
    pthread_mutex_unlock(&listHandler->lock);
    return ERR_NULL_POINTER;
}

/**
 *
 * @param listHandler
 * @param element
 * @param content
 * @param delete
 * @return
 */
err_t list_get_element(list_handler_t listHandler, chained_list_t* element, void* content, bool delete) {
    if (listHandler == NULL || content == NULL || element == NULL) {
        return ERR_PARAM;
    }
    memcpy(content, element->content, listHandler->contentLen);

    if (delete) {
        list_delete_element(listHandler, element);
    }
    return ERR_OK;
}

/**
 * @brief Get the current number of elements in the list
 *        (counted by traversal).
 *
 * @param listHandler List handler.
 * @return            Number of elements in the list (0 if empty).
 */
uint16_t list_get_size(list_handler_t listHandler) {
    uint16_t n = 0;

    pthread_mutex_lock(&listHandler->lock);
    chained_list_t* p = list_get_first_element(listHandler);
    if (p != NULL) {
        n = 1;
        while (p->next != NULL) {
            p = p->next;
            n++;
        }
    }
    pthread_mutex_unlock(&listHandler->lock);
    return n;
}

/**
 * @brief Insert a new element before or after a given element.
 *
 * @param listHandler List handler.
 * @param before      If true, insert before the reference element;
 *                    otherwise insert after.
 * @param place       Reference element for insertion.
 * @param content     Content to insert (copied internally).
 * @param id
 * @return            ERR_OK if success,
 *                    ERR_LIST_FULL if list is full,
 *                    ERR_NULL_POINTER if invalid pointers,
 *                    ERR_MALLOC_FAILED if memory allocation fails.
 */
err_t list_insert_element(list_handler_t listHandler, bool before, chained_list_t* place, const void* content, const uint64_t* id) {

    if (listHandler != NULL && place != NULL) {
        if (listHandler->listLength == LIST_MAX_LEN) {
            return ERR_LIST_FULL;
        }

        if (id != NULL && list_get_element_by_id_(listHandler, *id) != NULL) {
            return ERR_PARAM;
        }

        chained_list_t* p = malloc(sizeof(chained_list_t));
        if (p != NULL) {
            if (before) {
                if (place->prev != NULL) {
                    chained_list_t* prev = place->prev;
                    prev->next = p;
                    p->prev = prev;
                } else {
                    // Cas du premier
                    listHandler->head = p;
                }
                p->next = place;
                place->prev = p;
            } else {
                if (place->next != NULL) {
                    chained_list_t* next = place->next;
                    next->prev = p;
                    p->next = next;
                } else {
                    // Cas du dernier
                    listHandler->tail = p;
                }
                p->prev = place;
                place->next = p;
            }
            void* pNewContent = malloc(listHandler->contentLen);
            if (pNewContent != NULL) {
                memcpy(pNewContent, content, listHandler->contentLen);
                p->content = pNewContent;
                if (id) {
                    p->id = *id;
                } else {
                    p->id = ID_UNDEFINED;
                }
                listHandler->listLength++;
                return ERR_OK;
            }
        }
        return ERR_MALLOC_FAILED;
    }
    return ERR_NULL_POINTER;
}

/**
 * @brief Delete a specific element from the list and free its memory.
 *
 * @param listHandler List handler.
 * @param element     Element to delete.
 */
void list_delete_element(list_handler_t listHandler, chained_list_t* element) {
    if (listHandler->tail != NULL && element != NULL) {
        chained_list_t* prev = element->prev;
        chained_list_t* next = element->next;
        // Cas du premier
        if (prev == NULL && next != NULL) {
            next->prev = NULL;
            listHandler->head = next;
            // Cas du dernier
        } else if (prev != NULL && next == NULL) {
            prev->next = NULL;
            listHandler->tail = prev;
            // cas au milieux
        } else if (prev != NULL) {
            prev->next = next;
            next->prev = prev;
        } else {
            // Element seul
            listHandler->tail = NULL;
            listHandler->head = NULL;
        }
        listHandler->listLength--;
        free(element->content);
        free(element);
    }
}

/**
 * @brief Swap the position of two elements in the list.
 *
 * @param listHandler List handler.
 * @param a           First element.
 * @param b           Second element.
 * @return            ERR_OK if success, ERR_PARAM if invalid parameters.
 */
err_t list_swap_element(list_handler_t listHandler, chained_list_t* a, chained_list_t* b) {
    if (!a || !b || !listHandler) {
        return ERR_PARAM;
    }

    chained_list_t* tmp_prev_a = a->prev;
    chained_list_t* tmp_next_a = a->next;
    chained_list_t* tmp_prev_b = b->prev;
    chained_list_t* tmp_next_b = b->next;

    //printf("Swap info b[%x|%x|%x] c[%x|%x|%x] \n",a->prev,b,a->next,b->prev,c,b->next);
    // Si contigu
    if (tmp_next_a == b || tmp_prev_a == b) {
        if (tmp_next_a == b) {
            if (a->prev != NULL) {
                a->prev->next = b;
            }
            if (b->next != NULL) {
                b->next->prev = a;
            }
            a->next = tmp_next_b;
            a->prev = b;
            b->next = a;
            b->prev = tmp_prev_a;
        } else {
            if (b->prev != NULL) {
                b->prev->next = a;
            }
            if (a->next != NULL) {
                a->next->prev = b;
            }
            b->next = tmp_next_a;
            b->prev = a;
            a->next = b;
            a->prev = tmp_prev_b;
        }
    } else {
        if (a->prev != NULL) {
            a->prev->next = b;
        }
        if (b->prev != NULL) {
            b->prev->next = a;
        }
        if (a->next != NULL) {
            a->next->prev = b;
        }
        if (b->next != NULL) {
            b->next->prev = a;
        }
        a->next = tmp_next_b;
        a->prev = tmp_prev_b;
        b->next = tmp_next_a;
        b->prev = tmp_prev_a;
        // On replace head et tail
        if (a->next == NULL) {
            listHandler->tail = a;
        }
        if (b->next == NULL) {
            listHandler->tail = b;
        }
        if (a->prev == NULL) {
            listHandler->head = a;
        }
        if (b->prev == NULL) {
            listHandler->head = b;
        }
    }
    return ERR_OK;
}

chained_list_t* list_get_element_by_index_(list_handler_t listHandler, uint16_t index) {
    if (listHandler->head == NULL) {
        return NULL;
    }
    chained_list_t* elem = listHandler->head;
    uint8_t cnt = 0;
    while (elem->next != NULL && cnt < index) {
        elem = elem->next;
        cnt++;
    }
    return elem;
}

/**
 * @brief Retrieve an element by its index (counting from head).
 *
 * @param listHandler List handler.
 * @param index       Index of the element (0 = head).
 * @param content
 * @param delete      true : delete element after load
 * @return            Pointer to the element,
 *                    NULL if out of bounds or invalid handler.
 */
err_t list_get_element_by_index(list_handler_t listHandler, uint16_t index, void* content, bool delete) {
    pthread_mutex_lock(&listHandler->lock);
    if (!listHandler || !listHandler->head || index > listHandler->listLength || content == NULL) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_PARAM;
    }

    chained_list_t* elem = list_get_element_by_index_(listHandler, index);
    if (elem) {
        memcpy(content, elem->content, listHandler->contentLen);
        if (delete) {
            list_delete_element(listHandler, elem);
        }
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_OK;
    }

    pthread_mutex_unlock(&listHandler->lock);
    return ERR_NOT_FOUND;
}

chained_list_t* list_get_element_by_id_(list_handler_t listHandler, uint64_t id) {
    chained_list_t* p = listHandler->head;
    while (p != NULL) {
        if (p->id == id) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/**
 * @brief Retrieve an element by its index (counting from head).
 *
 * @param listHandler List handler.
 * @param id          id of the
 * @param content
 * @param delete      true : delete element after load
 * @return            Pointer to the element,
 *                    NULL if out of bounds or invalid handler.
 */
err_t list_get_element_by_id(list_handler_t listHandler, const uint64_t id, void* content, bool delete) {

    pthread_mutex_lock(&listHandler->lock);
    if (!listHandler || !listHandler->head || content == NULL || id == ID_UNDEFINED) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_PARAM;
    }

    chained_list_t* elem = list_get_element_by_id_(listHandler, id);
    if (elem) {
        memcpy(content, elem->content, listHandler->contentLen);
        if (delete) {
            list_delete_element(listHandler, elem);
        }
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_OK;
    }

    pthread_mutex_unlock(&listHandler->lock);
    return ERR_NOT_FOUND;
}

/**
 * @brief Search for a specific element in the list.
 *
 * @param listHandler     List handler.
 * @param searchedElement Element to look for.
 * @param result          Output boolean set to true if element is found.
 * @return                ERR_OK if success, ERR_NULL_POINTER if invalid pointer.
 */
err_t list_search_element(list_handler_t listHandler, const chained_list_t* searchedElement, bool* result) {
    *result = false;
    if (listHandler->tail != NULL) {
        chained_list_t* p = list_get_first_element(listHandler);
        while (p->next != NULL) {
            p = p->next;
            if (searchedElement == p) {
                *result = true;
                return ERR_OK;
            }
        }
        return ERR_OK;
    }
    return ERR_NULL_POINTER;
}