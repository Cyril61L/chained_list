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
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <malloc.h>
#include "stdio.h"
#include "stdint.h"






/**
 * @brief
 * @param list
 */
void init_list(chained_list_t *list) {
    list = NULL;
}

/**
 * @brief
 * @return
 */
chained_list_t *get_first_element(chained_list_t *list) {
    chained_list_t *p;
    if(list != NULL) {
        p = list;
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
chained_list_t *get_last_element(chained_list_t *list) {
    chained_list_t *p;
    if(list != NULL) {
        p = list;
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
chained_list_t *add(chained_list_t **list, uint8_t value) {
    chained_list_t *newElement = malloc(sizeof(chained_list_t));
    if(newElement != NULL) {
        if(*list == NULL) {
            printf("Premier élement %u\n",value);
            *list = newElement;
            newElement->x = value;
            newElement->next = NULL;
            newElement->prev = NULL;
            return newElement;
        } else {
            chained_list_t *p = get_last_element(*list);
            printf("Ajout élement %u\n",value);
            if(p != NULL) {
                newElement->x = value;
                p->next = newElement;
                newElement->prev = p;
                return newElement;
            }
        }
    }
    return NULL;
}


/**
 * @brief
 * @param list
 * @param value
 */
void pop(chained_list_t *list, uint8_t *value) {
    chained_list_t *p;
    chained_list_t *last = get_last_element(list);
    if(last != NULL) {
        printf("Delete %u\n",last->x);
        if(value!= NULL)*value = last->x;
        if(last->prev != NULL) {
            p = last->prev;
            p->next = NULL;
        }
        free(last);
    }
}

/**
 * @brief
 * @param list
 */
void show_list(chained_list_t *list) {
    chained_list_t *p = get_first_element(list);
    if(p != NULL) {
        printf("Show list ");
        while(p != NULL) {
            printf("->[%u]<-",p->x);
            p = p->next;
        }
    } else {
    }
    printf("\n");
}

/**
 * @brief
 * @param list
 * @return
 */
uint8_t get_size_list(chained_list_t *list) {
    uint8_t n = 0;
    chained_list_t *p = get_first_element(list);
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
chained_list_t *insert_element(chained_list_t *list,bool before,chained_list_t *place, uint8_t value) {
    if(list != NULL && place != NULL) {
        chained_list_t *p = malloc(sizeof(chained_list_t));
        if(p != NULL) {
            if(before) {
                if(place->prev != NULL) {
                    chained_list_t *prev = place->prev;
                    prev->next = p;
                    p->prev = prev;
                }
                p->next = place;
                place->prev = p;
            } else {
                if(place->next != NULL) {
                    chained_list_t *next = place->next;
                    next->prev = p;
                    p->next = next;
                }
                p->prev = place;
                place->next = p;
            }
            p->x = value;
            return p;
        }
    }
    return NULL;
}

/**
 * @brief
 * @param list
 * @param element
 * @return
 */
void *delete_element(chained_list_t *list,chained_list_t *element) {
    if(list != NULL && element != NULL) {
        chained_list_t *prev = element->prev;
        chained_list_t *next = element->next;
        // Cas du premier
        if(prev == NULL && next != NULL) {
            next->prev = NULL;
            // Cas du dernier
        } else if(prev != NULL && next == NULL) {
            prev->next = NULL;
            // cas au milieux
        } else if(prev != NULL) {
            prev->next = next;
            next->prev = prev;
        } else {
            // Element seul
        }
        free(element);
    }
}


/**
 * @brief swap two element
 * @param a
 * @param b
 */
void swap_element(chained_list_t *b, chained_list_t *c) {
    chained_list_t *tmp_prev_b = NULL,*tmp_prev_c = NULL,*tmp_next_b = NULL,*tmp_next_c = NULL;
    chained_list_t *a = NULL,*d = NULL;
    tmp_next_b = b->next;
    tmp_next_c = c->next;
    tmp_prev_b = b->prev;
    tmp_prev_c = c->prev;

    //printf("Swap info b[%x|%x|%x] c[%x|%x|%x] \n",b->prev,b,b->next,c->prev,c,c->next);
    // Si contigu
    if(tmp_next_b == c || tmp_prev_b == c) {
        if(tmp_next_b == c) {
            if(b->prev != NULL) {
                a = b->prev;
                a->next = c;
            }
            if(c->next != NULL) {
                d = c->next;
                d->prev = b;
            }
            b->next = tmp_next_c;
            b->prev = c;
            c->next = b;
            c->prev = tmp_prev_b;
        } else {
            if (c->prev != NULL) {
                a = c->prev;
                a->next = b;
            }
            if (b->next != NULL) {
                d = b->next;
                d->prev = c;
            }
            c->next = tmp_next_b;
            c->prev = b;
            b->next = c;
            b->prev = tmp_prev_c;
        }
    } else {
        chained_list_t *w,*x,*y,*z;
        if(b->prev != NULL) {
            w = b->prev;
            w->next = c;
        }
        if(c->prev != NULL) {
            x = c->prev;
            x->next = b;
        }
        if(b->next != NULL) {
            y = b->next;
            y->prev = c;
        }if(c->next != NULL) {
            z = c->next;
            z->prev = b;
        }
        b->next = tmp_next_c;
        b->prev = tmp_prev_c;
        c->next = tmp_next_b;
        c->prev = tmp_prev_b;
    }
}


/**
 * @brief
 * @param list
 */
void sort(chained_list_t *list) {
    bool swap = false;
    chained_list_t *next;
    chained_list_t *p = get_first_element(list);
    if(p != NULL && p->next != NULL) {
        while (p->next != NULL) {
            printf("Place : %u\n",p->x);
            // Step 1 :
            next = p->next;
            while(p->x > next->x) {
                swap = true;
                swap_element(p,next);
                show_list(list);
                next = p->next;
                if(next == NULL)
                    break;
            }
            if(swap == true) {
                p = get_first_element(list);
                swap = false;
            } else {
                p = p->next;
            }
        }
    }
}


