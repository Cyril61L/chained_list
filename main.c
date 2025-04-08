#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chained_list.h"

chained_list_t *mylist = NULL;

int main(void) {
    printf("Hello, World!\n");

    // On commence par initialiser le générateur de nombre pseudo-aléatoires.
    srand( time( NULL ) );

    init_list(mylist);

    for(uint8_t i=16;i>0;i--) {
        add(&mylist,rand()%128);
    }

    show_list(mylist);
    sort(mylist);
    show_list(mylist);



    return 0;
}
