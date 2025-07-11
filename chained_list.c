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
#include "stdio.h"
#include "stdint.h"




err_t search_element(list_handler_t *listHandler, chained_list_t *searchedElement, bool *result);


err_t list_init(list_handler_t *listHandler, const char *name, size_t len);
uint16_t list_get_size(list_handler_t *listHandler);

err_t list_add(list_handler_t *listHandler, void *content);
err_t list_pop(list_handler_t *listHandler, void *content);

err_t list_insert_element(list_handler_t *listHandler,bool before,chained_list_t *place, void *content);
chained_list_t *list_get_first_element(list_handler_t *listHandler);
chained_list_t *list_get_last_element(list_handler_t *listHandler);
void list_delete_element(list_handler_t *listHandler,chained_list_t *element);
err_t list_swap_element(chained_list_t *b, chained_list_t *c);

/**
 * @brief
 * @param list
 */
err_t list_init(list_handler_t *listHandler, const char *name, size_t len) {
    if(len > CONTENT_MAX_SIZE || len <= 0) {
        return ERR_PARAM;
    }
    strncpy((char *) listHandler->name, name, LIST_NAME_SIZE);
    listHandler->head = NULL;
    listHandler->tail = NULL;
    listHandler->contentLen = len;
    listHandler->listLength = 0;
    return ERR_OK;
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

    chained_list_t *newElement = malloc(sizeof(chained_list_t));
    if(newElement != NULL) {
        if(listHandler->tail == NULL) {
            listHandler->tail = newElement;
            listHandler->head = listHandler->tail;
            void *p = malloc(listHandler->contentLen);
            if(p != NULL) {
                memcpy(p,content,listHandler->contentLen);
                newElement->content = p;
                newElement->next = NULL;
                newElement->prev = NULL;
                listHandler->listLength++;
                return ERR_OK;
            } else {
                free(newElement);
                return ERR_MALLOC_FAILED;
            }
        } else {
            chained_list_t *pLast = listHandler->tail;//list_get_last_element(listHandler);
            void *p = malloc(listHandler->contentLen);
            if (p != NULL) {
                memcpy(p, content, listHandler->contentLen);
                pLast->next = newElement;
                newElement->content = p;
                newElement->prev = pLast;
                newElement->next = NULL;
                // move head and tail
                listHandler->tail = newElement;
                listHandler->head = listHandler->tail;
                listHandler->listLength++;
                return ERR_OK;
            } else {
                free(newElement);
                return ERR_MALLOC_FAILED;
            }
        }
        return ERR_NULL_POINTER;
    }
    return ERR_MALLOC_FAILED;
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

    chained_list_t *last = listHandler->tail;//list_get_last_element(listHandler);
    if(last != NULL) {
        if(last->content != NULL) {
            memcpy(content,last->content,listHandler->contentLen);
            free(last->content);
        }
        if(last->prev != NULL) {
            last->prev->next = NULL;
            listHandler->tail = last->prev;
        } else {
            // On supprimait le seul élément
            listHandler->tail = NULL;
            listHandler->head = NULL;
        }
        free(last);
        listHandler->listLength--;
        return ERR_OK;
    }
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
err_t list_swap_element(chained_list_t *a, chained_list_t *b) {
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
    }
    return ERR_OK;
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