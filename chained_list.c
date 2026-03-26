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

chained_list_t* list_get_first_element(list_handler_t* listHandler);
chained_list_t* list_get_last_element(list_handler_t* listHandler);
err_t list_insert_element(list_handler_t* listHandler, bool before, chained_list_t* place, const void* content, const uint64_t* id);
err_t list_get_element_(list_handler_t* listHandler, chained_list_t* element, void* content, bool delete);
err_t list_delete_element(list_handler_t* listHandler, chained_list_t* element);
err_t list_swap_element(list_handler_t* listHandler, chained_list_t* a, chained_list_t* b);
chained_list_t* list_get_element_by_id_(list_handler_t* listHandler, uint64_t id);
chained_list_t* list_get_element_by_index_(list_handler_t* listHandler, uint16_t index);

/**
 * @brief Initialize a new linked list handler.
 *
 * @param listHandler Pointer to the list handler to initialize.
 * @param len         Size in bytes of each element’s content.
 * @param mode        List operating mode (FIFO or LIFO).
 * @return            ERR_OK if success, ERR_PARAM if invalid parameters.
 */
err_t list_init(list_handler_t* listHandler, size_t len, list_mode_e mode) {
    if (len > CONTENT_MAX_SIZE || len <= 0) {
        return ERR_PARAM;
    }

    listHandler->head = NULL;
    listHandler->tail = NULL;
    listHandler->listMode = mode;
    listHandler->contentLen = len;
    listHandler->listLength = 0;

    pthread_mutex_init(&listHandler->lock, NULL);
    return ERR_OK;
}

/**
 * @brief Free all memory allocated for the list and its elements.
 *
 * @param listHandler Pointer to the list handler to free.
 * @return            ERR_OK if success, ERR_NULL_POINTER if pointer is invalid.
 */
err_t list_free(list_handler_t* listHandler) {

    if (listHandler != NULL) {
        chained_list_t* elem;
        pthread_mutex_lock(&listHandler->lock);
        do {
            elem = list_get_last_element(listHandler);
            list_delete_element(listHandler, elem);
        } while (elem != NULL);
        pthread_mutex_unlock(&listHandler->lock);
        pthread_mutex_destroy(&listHandler->lock);
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
chained_list_t* list_get_first_element(list_handler_t* listHandler) {
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
chained_list_t* list_get_last_element(list_handler_t* listHandler) {
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
err_t list_add(list_handler_t* listHandler, const void* content) {
    return list_add_id(listHandler, content, ID_UNDEFINED);
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
err_t list_add_id(list_handler_t* listHandler, const void* content, const uint64_t id) {
    if (listHandler == NULL || content == NULL) {
        return ERR_PARAM;
    }
    pthread_mutex_lock(&listHandler->lock);
    if (listHandler->listLength == LIST_MAX_LEN) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_LIST_FULL;
    }

    if (id != ID_UNDEFINED && list_get_element_by_id_(listHandler, id) != NULL) {
        pthread_mutex_unlock(&listHandler->lock);
        return ERR_PARAM;
    }

    // Allocate new node and content
    chained_list_t* newElement = malloc(sizeof(chained_list_t));
    memset(newElement,0x00,sizeof(chained_list_t));
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
    newElement->id = id;
    if (listHandler->tail && listHandler->head) {
        if (listHandler->listMode == LIST_MODE_FIFO) {
            newElement->next = listHandler->head;
            newElement->prev = NULL;
            listHandler->head->prev = newElement;
            listHandler->head = newElement;
        } else {
            newElement->prev = listHandler->tail;
            newElement->next = NULL;
            listHandler->tail->next = newElement;
            listHandler->tail = newElement;
        }
    } else {
        // First element
        listHandler->head = newElement;
        listHandler->tail = newElement;
        listHandler->current = newElement;
    }

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
err_t list_pop(list_handler_t* listHandler, void* content) {

    if (listHandler == NULL || content == NULL || listHandler->tail == NULL) {
        return ERR_PARAM;
    }
    // Lock
    pthread_mutex_lock(&listHandler->lock);
    err_t ret = list_get_element_(listHandler,listHandler->tail,content,true);
    if (ret != ERR_OK) {
        pthread_mutex_unlock(&listHandler->lock);
        return ret;
    }
    pthread_mutex_unlock(&listHandler->lock);
    return ERR_OK;
}

/**
 *
 * @param listHandler
 * @param element
 * @param content
 * @param delete
 * @return
 */
err_t list_get_element_(list_handler_t* listHandler, chained_list_t* element, void* content, bool delete) {
    if (listHandler == NULL || content == NULL) {
        return ERR_PARAM;
    }

    if (element == NULL) {
        return ERR_NOT_FOUND;
    }

    memcpy(content, element->content, listHandler->contentLen);
    if (delete) {
        return list_delete_element(listHandler, element);
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
uint16_t list_get_size(list_handler_t* listHandler) {
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
err_t list_insert_element(list_handler_t* listHandler, bool before, chained_list_t* place, const void* content, const uint64_t* id) {

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
err_t list_delete_element(list_handler_t* listHandler, chained_list_t* element) {
    if (listHandler->tail != NULL && element != NULL) {
        chained_list_t* prev = element->prev;
        chained_list_t* next = element->next;
        // Cas du premier
        if (prev == NULL && next != NULL) {
            next->prev = NULL;
            listHandler->head = next;
            if (listHandler->current == element) {
                listHandler->current = next;
            }
            // Cas du dernier
        } else if (prev != NULL && next == NULL) {
            prev->next = NULL;
            listHandler->tail = prev;
            if (listHandler->current == element) {
                listHandler->current = prev;
            }
            // cas au milieux
        } else if (prev != NULL) {
            prev->next = next;
            next->prev = prev;
            if (listHandler->current == element) {
                listHandler->current = prev;
            }
        } else {
            // Element seul
            listHandler->tail = NULL;
            listHandler->head = NULL;
            listHandler->current = NULL;
        }
        listHandler->listLength--;
        free(element->content);
        free(element);
    } else {
        return ERR_NULL_POINTER;
    }
    return ERR_OK;
}

/**
 * @brief Swap the position of two elements in the list.
 *
 * @param listHandler List handler.
 * @param a           First element.
 * @param b           Second element.
 * @return            ERR_OK if success, ERR_PARAM if invalid parameters.
 */
err_t list_swap_element(list_handler_t* listHandler, chained_list_t* a, chained_list_t* b) {
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
    }
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
    return ERR_OK;
}

chained_list_t* list_get_element_by_index_(list_handler_t* listHandler, uint16_t index) {
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
err_t list_get_element_by_index(list_handler_t* listHandler, uint16_t index, void* content, bool delete) {
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

chained_list_t* list_get_element_by_id_(list_handler_t* listHandler, uint64_t id) {
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
err_t list_get_element_by_id(list_handler_t* listHandler, const uint64_t id, void* content, bool delete) {

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
 *
 * @param listHandler
 * @param move
 * @param content
 * @return
 */
err_t list_browse(list_handler_t* listHandler, list_move_e move, void* content)
{
    if (listHandler == NULL || listHandler->head == NULL) {
        return ERR_NULL_POINTER;
    }

    err_t ret = ERR_OK;
    pthread_mutex_lock(&listHandler->lock);
    switch (move) {
    case FIRST_ELEM:
        ret = list_get_element_(listHandler, listHandler->head, content, false);
        break;
    case LAST_ELEM:
        ret = list_get_element_(listHandler, listHandler->tail, content, false);
        break;
    case CURRENT_ELEM:
        ret = list_get_element_(listHandler, listHandler->current, content, true);
        break;
    case NEXT_ELEM:
        ret = list_get_element_(listHandler, listHandler->current->next, content, false);
        if (ret == ERR_OK) {
            listHandler->current = listHandler->current->next;
        } else if (ret == ERR_NOT_FOUND && listHandler->listMode == LIST_MODE_CIRCULAR) {
            listHandler->current = listHandler->head;
            ret = list_get_element_(listHandler, listHandler->current, content, false);
        }
        break;
    case PREV_ELEM:
        ret = list_get_element_(listHandler, listHandler->current->prev, content, false);
        if (ret == ERR_OK) {
            listHandler->current = listHandler->current->prev;
        } else if (ret == ERR_NOT_FOUND && listHandler->listMode == LIST_MODE_CIRCULAR) {
            listHandler->current = listHandler->tail;
            ret = list_get_element_(listHandler, listHandler->current, content, false);
        }
        break;
    default:
        ret = ERR_NOT_FOUND;
    }
    pthread_mutex_unlock(&listHandler->lock);
    return ret;
}

/**
 * @brief Search for a specific element in the list.
 *
 * @param listHandler     List handler.
 * @param searchedElement Element to look for.
 * @param result          Output boolean set to true if element is found.
 * @return                ERR_OK if success, ERR_NULL_POINTER if invalid pointer.
 */
err_t list_search_element(list_handler_t* listHandler, const chained_list_t* searchedElement, bool* result) {
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

void split(chained_list_t * source, chained_list_t** front, chained_list_t** back) {
    chained_list_t* slow = source;
    chained_list_t* fast = source->next;

    while (fast) {
        fast = fast->next;
        if (fast) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *front = source;
    *back = slow->next;
    slow->next = NULL;
}

chained_list_t* fusion(chained_list_t* a, chained_list_t* b) {
    if (!a) return b;
    if (!b) return a;

    chained_list_t* result = NULL;
    if (a->id <= b->id) {
        result = a;
        result->next = fusion(a->next, b);
        if (result->next) result->next->prev = result;  // 🔥 important
    } else {
        result = b;
        result->next = fusion(a, b->next);
        if (result->next) result->next->prev = result;  // 🔥 important
    }
    result->prev = NULL;  // sécurité
    return result;
}

chained_list_t* mergeSort(chained_list_t* head) {
    if (!head || !head->next) return head;

    chained_list_t *a, *b;
    split(head, &a, &b);

    a = mergeSort(a);
    b = mergeSort(b);

    return fusion(a, b);
}

/**
 * @brief Sort by id using merge sort method
 * @param listHandler
 * @return
 */
err_t list_sort_by_id(list_handler_t* listHandler)
{
    pthread_mutex_lock(&listHandler->lock);
    listHandler->head = mergeSort(listHandler->head);
    listHandler->tail = list_get_last_element(listHandler);
    pthread_mutex_unlock(&listHandler->lock);
    return ERR_OK;
}

/**
 * @brief
 * @param list
 */
void list_slow_sort(list_handler_t *list) {
    bool swap = false;
    chained_list_t *next;
    chained_list_t *p = list->head;
    if(p != NULL && p->next != NULL) {
        while (p->next != NULL) {
            //printf("Place : %u\n",p->x);
            // Step 1 :
            next = p->next;
            while(p->id > next->id) {
                swap = true;
                list_swap_element(list,p,next);
                //list_print(list);
                next = p->next;
                if(next == NULL)
                    break;
            }
            if(swap == true) {
                p = list->head;
                swap = false;
            } else {
                p = p->next;
            }
        }
    }
}


/**
 * Sort using selection method
 * @param list
 */
void list_insertion_sort(list_handler_t *list) {
    chained_list_t *sorted_end = list->head;

    while (sorted_end->next) {
        chained_list_t *current = sorted_end->next;

        // Cherche la position en remontant à rebours — O(k) au lieu de repartir du début
        chained_list_t *pos = sorted_end;
        while (pos->prev != NULL && pos->id > current->id) {
            pos = pos->prev;
        }

        // current est déjà à la bonne place
        if (pos == sorted_end && pos->id <= current->id) {
            sorted_end = sorted_end->next;
            continue;
        }

        // Détache current
        sorted_end->next = current->next;
        if (current->next) current->next->prev = sorted_end;

        if (pos->id > current->id) {
            // Insertion en tête
            current->next = pos;
            current->prev = NULL;
            pos->prev = current;
            list->head = current;
        } else {
            // Insertion après pos
            current->next = pos->next;
            current->prev = pos;
            if (pos->next) pos->next->prev = current;
            pos->next = current;
        }
        // sorted_end ne bouge pas : on réinsère avant lui
    }
    list->tail = sorted_end;
}


chained_list_t *get_higher_element(list_handler_t *list) {
    if(list != NULL) {
        chained_list_t *p = list->head;
        chained_list_t *higher = p;
        while(p != NULL) {
            if(p->id > higher->id)
                higher = p;
            if(p->next == NULL)
                break;
            p = p->next;
        }
        return higher;
    }
    return NULL;
}

/**
 * @brief Sort using selection method
 * @param list
 */
void list_selection_sort(list_handler_t *list) {
    chained_list_t *higher,*tmp = NULL;

    for(uint32_t i=0; i<list->listLength-1;i++) {
        // Recherche l'element avec la plus grande valeur
        // [15]---[2]---[8]---[12] => [15]
        higher = get_higher_element(list);
        // Place le plus haut element trouvé en dernier
        // [12]---[2]---[8]---[15]
        list_swap_element(list, higher, list->tail);
        // On décroche le dernier element pour ne pas le compter dans la prochaine recherche de la plus haute valeur
        // [12]---[2]---[8]-x-[15]
        list->tail = higher->prev;
        higher->prev->next = NULL;
        if(tmp != NULL)
            // On raccroche la suite à partir du second tour
                // [8]---[2]-x-[12]-x-[15] => [8]---[2]-x-[12]---[15]
                    higher->next = tmp;
        tmp = higher;
        //show_list(list);
    }
    chained_list_t *first = list->head;
    // On raccroche le reste
    // [2]-x-[8]---[12]---[15] => [2]---[8]---[12]---[15]
    first->next = higher;
    list->tail = list_get_last_element(list);
}

err_t list_print(list_handler_t* listHandler)
{
    if (listHandler == NULL && listHandler->head == NULL)
        return ERR_NULL_POINTER;

    chained_list_t *p = listHandler->head;
    if(p != NULL) {
        printf("Show list ");
        while(p != NULL) {
            printf("->[%u]<-",p->id);
            p = p->next;
        }
    } else {
    }
    printf("\n");
    return ERR_OK;
}