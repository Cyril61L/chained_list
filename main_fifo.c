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
        list_add(&myFifo,&msgTab[i]);
    }

    printf("len %u\n", list_get_size(&myFifo));

    if(list_swap_element(&myFifo,list_get_first_element(&myFifo)->next->next, list_get_first_element(&myFifo)->next->next->next)) {
        printf("Error");
    }


    // Depiler la liste en mode fifo
    for(uint8_t i=0;i<51;i++) {
        msg_t get;
        err_t err = list_pop(&myFifo, &get);
        if(err) {
            printf("Error get msg from list %u\n",err);
        } else {
            printf("Get %s : %u\n",get.text,get.cmd);
        }
    }

    return 0;
}