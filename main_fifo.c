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

    msg_t msgTab[4];

    create_msg("Coucou msg 1",1,&msgTab[0]);
    create_msg("Coucou msg 2",2,&msgTab[1]);
    create_msg("Coucou msg 3",3,&msgTab[2]);
    create_msg("Coucou msg 4",4,&msgTab[3]);

    if(list_init(&myFifo,"Mylist", sizeof(msg_t))) {
        printf("Error init list\n");
    }

    // Ajout message dans liste chainee en tete pour fifo
    for(uint8_t i=0;i<105;i++) {
        insert_first(&msgTab[i%2]);
    }

    printf("len %u\n", list_get_size(&myFifo));

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