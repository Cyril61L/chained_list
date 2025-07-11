#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include "chained_list.h"
#include "lib_uart.h"
#include "lib_log.h"
#include "options_protocol.h"


#define RS_485_COM "/dev/ttyLP4"
list_handler_t mylist;


#define MQ_NAME "/RX_queue_2"
#define MAX_SIZE 256

handle_uart_t handleUart;

void print_hex(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

/*
void *reader_thread(void *arg) {
    mqd_t mq;
    char buffer[MAX_SIZE];
    ssize_t bytes_read;

    mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open (reader)");
        pthread_exit(NULL);
    }

    while (1) {
        memset(buffer, 0, MAX_SIZE);
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0) {
            printf("Message reçu (%ld octets) : ", bytes_read);
            print_hex((unsigned char *)buffer, bytes_read);
            uart_tx(&handleUart,"ok",3);
        } else {
            perror("mq_receive");
            break;
        }
    }

    mq_close(mq);
    pthread_exit(NULL);
}


typedef enum {
    RCST_REQUEST = 0x01,
    RCST_REPLY = 0x02,
    UPDATE_COMMON_CFG = 0x03,
    UPDATE_PERMANENT_CFG = 0x04,
    UPDATE_GMT8TIME = 0x05,
    ALIVE = 0x06,
    ACK = 0x40,
    NACK = 0x46,
    NONE = 0x07,
    CMD_RELAY = 0x08,
    PULSE = 0x09,
    CMD_SYNC_WIRED = 0x0A
}options_cmd_e;
 */

typedef struct {
    uint8_t cmd;
    char text[32];
}msg_t;

void create_msg(const char*text, uint8_t cmd, msg_t *msg) {
    memcpy(msg->text,text, sizeof(msg->text));
    msg->cmd = cmd;
}

int main(void) {
    printf("Exemple d'utilisation de liste chainée en lifo\n");
    msg_t msgTab[4];

    create_msg("Coucou msg 1",1,&msgTab[0]);
    create_msg("Coucou msg 2",2,&msgTab[1]);
    create_msg("Coucou msg 3",3,&msgTab[2]);
    create_msg("Coucou msg 4",4,&msgTab[3]);

    if(list_init(&mylist,"Mylist", sizeof(msg_t))) {
        printf("Error init list\n");
    }

    // Ajout message dans liste chainee
    for(uint8_t i=0;i<4;i++) {
        if(list_add(&mylist,&msgTab[i])) {
            printf("Error add msg to list\n");
        } else {
            printf("Add msg %u to list\n",i);
        }
    }


    // Depiler la liste en mode last in first out
    for(uint8_t i=0;i<4;i++) {
        msg_t get;
        if(list_pop(&mylist, &get)) {
            printf("Error get msg from list\n");
        } else {
            printf("Msg %s : %u\n",get.text,get.cmd);
        }
    }




/*



    if(uart_open(&handleUart,RS_485_COM,B38400,MQ_NAME)) {
        LOG_INFO("Open %s success",RS_485_COM);
    }
    else {
        LOG_ERROR("Open %s failed",RS_485_COM);
        return 0;
    }

    sleep(2);
    uart_tx(&handleUart,"Hello world\n",strlen("Hello world\n"));

    // Lancer le thread lecteur
    if (pthread_create(&thread, NULL, reader_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    else {
        LOG_INFO("Create RX thread");
    }

    // Lancer le thread lecteur
    if (pthread_create(&thread, NULL, reader_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    else {
        LOG_INFO("Create RX thread");
    }

    while (1) {

    }

*/

    return 0;
}
