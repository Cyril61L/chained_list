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
#include "stdint.h"





/**
 * @brief
 * @param list
 */
err_t list_init(list_handler_t *listHandler, const char *name, size_t len, list_mode_t mode) {
    if(len > CONTENT_MAX_SIZE || len <= 0) {
        return ERR_PARAM;
    }
    strncpy((char *) listHandler->name, name, LIST_NAME_SIZE);
    listHandler->head = NULL;
    listHandler->tail = NULL;
    listHandler->listMode = mode;
    listHandler->contentLen = len;
    listHandler->listLength = 0;
    pthread_mutex_init(&listHandler->lock, NULL);
    return ERR_OK;
}


/**
 * @brief
 * @param listHandler
 * @return
 */
err_t list_free(list_handler_t *listHandler) {
    chained_list_t *elem;
    if(listHandler != NULL) {
        pthread_mutex_lock(&listHandler->lock);
        do {
            elem = list_get_last_element(listHandler);
            list_delete_element(listHandler,elem);
        } while (elem != NULL);
        pthread_mutex_unlock(&listHandler->lock);
        pthread_mutex_destroy(&listHandler->lock);
        listHandler = NULL;
        return ERR_OK;
    }
    return ERR_NULL_POINTER;
}


/**
 * @brief
 * @return
 */
chained_list_t *list_get_first_element(list_handler_t *listHandler) {
    chained_list_t *p;
    if(listHandler->tail != NULL) {
        p = listHandler->tail;
        while(p->prev != NULL) {
            p = p->prev;
        }
        return p;
    }
    return NULL;
}


/**
 * @brief
 * @return
 */
chained_list_t *list_get_last_element(list_handler_t *listHandler) {
    chained_list_t *p;
    if(listHandler->head != NULL) {
        p = listHandler->head;
        while(p->next != NULL) {
            p = p->next;
        }
        return p;
    }
    return NULL;
}





/**
 * @brief
 * @param list
 * @param value
 * @return
 */
err_t list_add(list_handler_t *listHandler, void *content) {
    if(listHandler == NULL || content == NULL) {
        return ERR_PARAM;
    }

    if(listHandler->listLength == LIST_MAX_LEN ) {
        return ERR_LIST_FULL;
    }
    // Lock
    pthread_mutex_lock(&listHandler->lock);
    // Allocate new node and content
    chained_list_t *newElement = malloc(sizeof(chained_list_t));
    if (newElement == NULL) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_MALLOC_FAILED;
    }

    void *p = malloc(listHandler->contentLen);
    if (p == NULL) {
        free(newElement);
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_MALLOC_FAILED;
    }
    memcpy(p, content, listHandler->contentLen);
    newElement->content = p;
    newElement->next = NULL;
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
 * @brief
 * @param list
 * @param value
 */
err_t list_pop(list_handler_t *listHandler, void *content) {
    chained_list_t *p;

    if(listHandler == NULL || content == NULL || listHandler->tail == NULL) {
        return ERR_PARAM;
    }
    // Lock
    pthread_mutex_lock(&listHandler->lock);

    if(listHandler->listMode == LIST_MODE_FIFO) {
        p = listHandler->head;
    } else {
        p = listHandler->tail;
    }
    if(p != NULL) {
        if(p->content != NULL) {
            memcpy(content,p->content,listHandler->contentLen);
            free(p->content);
        }
        if(listHandler->listMode == LIST_MODE_FIFO && p->next != NULL) {
            p->next->prev = NULL;
            listHandler->head = p->next;
        } else if(listHandler->listMode == LIST_MODE_LIFO && p->prev != NULL) {
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
 * @brief
 * @param list
 * @return
 */
uint16_t list_get_size(list_handler_t *listHandler) {
    uint16_t n = 0;
    chained_list_t *p = list_get_first_element(listHandler);
    if(p != NULL) {
        n = 1;
        while(p->next != NULL) {
            p = p->next;
            n++;
        }
    }
    return n;
}

/**
 * @brief
 * @param list
 * @param before
 * @param place
 * @param value
 * @return
 */
err_t list_insert_element(list_handler_t *listHandler,bool before,chained_list_t *place, void *content) {

    if(listHandler != NULL && place != NULL) {
        if(listHandler->listLength == LIST_MAX_LEN) {
            return ERR_LIST_FULL;
        }

        chained_list_t *p = malloc(sizeof(chained_list_t));
        if(p != NULL) {
            if(before) {
                if(place->prev != NULL) {
                    chained_list_t *prev = place->prev;
                    prev->next = p;
                    p->prev = prev;
                } else {
                    // Cas du premier
                    listHandler->head = p;
                }
                p->next = place;
                place->prev = p;
            } else {
                if(place->next != NULL) {
                    chained_list_t *next = place->next;
                    next->prev = p;
                    p->next = next;
                } else {
                    // Cas du dernier
                    listHandler->tail = p;
                }
                p->prev = place;
                place->next = p;
            }
            void *pNewContent = malloc(listHandler->contentLen);
            if(pNewContent != NULL) {
                memcpy(pNewContent, content,listHandler->contentLen);
                p->content = pNewContent;
                listHandler->listLength++;
                return ERR_OK;
            }
        }
        return ERR_MALLOC_FAILED;
    }
    return ERR_NULL_POINTER;
}

/**
 * @brief
 * @param list
 * @param element
 * @return
 */
void list_delete_element(list_handler_t *listHandler,chained_list_t *element) {
    if(listHandler->tail != NULL && element != NULL) {
        chained_list_t *prev = element->prev;
        chained_list_t *next = element->next;
        // Cas du premier
        if(prev == NULL && next != NULL) {
            next->prev = NULL;
            listHandler->head = next;
            // Cas du dernier
        } else if(prev != NULL && next == NULL) {
            prev->next = NULL;
            listHandler->tail = prev;
            // cas au milieux
        } else if(prev != NULL) {
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
 * @brief swap two element
 * @param a
 * @param b
 */
err_t list_swap_element(list_handler_t *listHandler,chained_list_t *a, chained_list_t *b) {
    if(!a || !b || !listHandler) {
        return ERR_PARAM;
    }

    chained_list_t *tmp_prev_a = a->prev;
    chained_list_t *tmp_next_a = a->next;
    chained_list_t *tmp_prev_b = b->prev;
    chained_list_t *tmp_next_b = b->next;

    //printf("Swap info b[%x|%x|%x] c[%x|%x|%x] \n",a->prev,b,a->next,b->prev,c,b->next);
    // Si contigu
    if(tmp_next_a == b || tmp_prev_a == b) {
        if(tmp_next_a == b) {
            if(a->prev != NULL) {
                a->prev->next = b;
            }
            if(b->next != NULL) {
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
        if(a->prev != NULL) {
            a->prev->next = b;
        }
        if(b->prev != NULL) {
            b->prev->next = a;
        }
        if(a->next != NULL) {
            a->next->prev = b;
        }if(b->next != NULL) {
            b->next->prev = a;
        }
        a->next = tmp_next_b;
        a->prev = tmp_prev_b;
        b->next = tmp_next_a;
        b->prev = tmp_prev_a;
        // On replace head et tail
        if(a->next == NULL) {
            listHandler->tail = a;
        }
        if(b->next == NULL) {
            listHandler->tail = b;
        }
        if(a->prev == NULL) {
            listHandler->head = a;
        }
        if(b->prev == NULL) {
            listHandler->head = b;
        }
    }
    return ERR_OK;
}


/**
 * @brief Count from head to tail and return the element index.
 * @param listHandler
 * @param index
 * @return
 */
chained_list_t *list_get_element_by_index(list_handler_t *listHandler, uint16_t index) {
    if(!listHandler || !listHandler->head || index > listHandler->listLength) {
        return NULL;
    } else {
        chained_list_t *p = listHandler->head;
        uint8_t cnt = 0;
        while(p->next != NULL && cnt < index) {
            p = p->next;
            cnt++;
        }
        return p;
    }
}


/**
 * @brief
 * @param listHandler
 * @param searchedElement
 * @return
 */
err_t list_search_element(list_handler_t *listHandler, chained_list_t *searchedElement, bool *result) {
    *result = false;
    if(listHandler->tail != NULL) {
        chained_list_t  *p = list_get_first_element(listHandler);
        while(p->next != NULL) {
                p = p->next;
                if(searchedElement == p) {
                    *result = true;
                    return ERR_OK;
                }
        }
        return ERR_OK;
    }
    return ERR_NULL_POINTER;
}