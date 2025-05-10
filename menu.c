#include "menu.h"
#include <stdio.h>

void menu(int * choix ) {
    do {
        printf("\n=======[ MENU ]=======\n 0) Partie locale\n 1) Creer une LAN\n 2) Rejoindre une LAN\n 3) Quitter\n======================\n[X]==>");
        scanf("%d", choix);  printf("\n");
    } while (*choix<0 || *choix>3);
}