/**
 * @file      main_fifi.c
 * @authors   B06V10013
 * @date      11/07/2025
 * @copyright 2025 ${ORGANIZATION_NAME}. All rights reserved.
 *
 * @brief 
 */

#include <stdio.h>
#include <string.h>
#include "chained_list.h"

list_handler_t myFifo;

typedef struct {
    uint8_t cmd;
    char text[32];
}msg_t;

void create_msg(const char*text, uint8_t cmd, msg_t *msg) {
    memcpy(msg->text,text, sizeof(msg->text));
    msg->cmd = cmd;
}


void insert_first(msg_t *msg) {
        chained_list_t *firstEl = list_get_first_element(&myFifo);
        if(firstEl != NULL) {
            if(list_insert_element(&myFifo,true,firstEl,msg)) {
                printf("Error insert to list\n");
            } else {
                printf("Insert %s : %u\n",msg->text, msg->cmd);
            }
        } else {
            if(list_add(&myFifo,msg)) {
                printf("Error add to list\n");
            } else {
                printf("Insert %s : %u\n",msg->text, msg->cmd);
            }
        }
}

int main(void) {
    printf("Exemple d'utilisation de liste chainée en fifo\n");

    msg_t msgTab[50];

    if(list_init(&myFifo,"Mylist", sizeof(msg_t))) {
        printf("Error init list\n");
    }

    for(uint8_t i=0;i<50;i++) {
        char buffer[32];
        sprintf(buffer,"Coucou msg %u", i);
        create_msg(buffer, i, &msgTab[i]);
    }

    // Ajout message dans liste chainee en tete pour fifo
    for(uint8_t i=0;i<50;i++) {
        insert_first(&msgTab[i]);
    }

    printf("len %u\n", list_get_size(&myFifo));

    list_swap_element(&myFifo,list_get_first_element(&myFifo)->next, list_get_first_element(&myFifo)->next->next->next);

    chained_list_t *element = list_get_element_by_index(&myFifo,25);
    msg_t *msg25 = element->content;
    printf("ele 25 %s %u\n",msg25->text,msg25->cmd);

    // Depiler la liste en mode
    for(uint8_t i=0;i<4;i++) {
        msg_t get;
        if(list_pop(&myFifo, &get)) {
            printf("Error get msg from list\n");
        } else {
            printf("Get %s : %u\n",get.text,get.cmd);
        }
    }

    return 0;
}