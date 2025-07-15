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


typedef struct {
    uint8_t cmd;
    char text[32];
}msg_t;

void create_msg(const char*text, uint8_t cmd, msg_t *msg) {
    memcpy(msg->text,text, sizeof(msg->text));
    msg->cmd = cmd;
}

list_handler_t myFifo;
list_handler_t myLifo;

int main(void) {
    printf("Exemple d'utilisation de liste chainée\n");


    msg_t msgTabFifo[10];
    msg_t msgTabLifo[10];


    if(list_init(&myFifo,"Fifo", sizeof(msg_t),LIST_MODE_FIFO)) {
        printf("Error init list\n");
    }
    if(list_init(&myLifo,"Lifo", sizeof(msg_t),LIST_MODE_LIFO)) {
        printf("Error init list\n");
    }

    for(uint8_t i=0;i<10;i++) {
        char buffer[32];
        sprintf(buffer,"Fifo msg %u", i);
        create_msg(buffer, i, &msgTabFifo[i]);
        sprintf(buffer,"Lifo msg %u", i);
        create_msg(buffer, i, &msgTabLifo[i]);

    }

    // Ajout message dans liste chainee en tete pour fifo
    for(uint8_t i=0;i<10;i++) {
        err_t err = list_add(&myFifo,&msgTabFifo[i]);
        list_add(&myLifo,&msgTabLifo[i]);
    }


    printf("---------------------%s--------------------------\n",myFifo.name);
    // Depiler la liste en mode fifo
    for(uint8_t i=0;i<10;i++) {
        msg_t get;
        err_t err = list_pop(&myFifo, &get);
        if(err) {
            printf("Error get msg from list %u\n",err);
        } else {
            printf("Get %s : %u\n",get.text,get.cmd);
        }
    }

    printf("---------------------%s--------------------------\n",myLifo.name);

    // Depiler la liste en mode Lifo
    for(uint8_t i=0;i<10;i++) {
        msg_t get;
        err_t err = list_pop(&myLifo, &get);
        if(err) {
            printf("Error get msg from list %u\n",err);
        } else {
            printf("Get %s : %u\n",get.text,get.cmd);
        }
    }

    return 0;
}