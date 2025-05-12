#include "menu.h"
#include <stdio.h>
#include "GRAPHISMES/graphismes.h"

/*
void menu(int * choix ) {
    do {
        printf("\n=======[ MENU ]=======\n 0) Partie locale\n 1) Creer une LAN\n 2) Rejoindre une LAN\n 3) Quitter\n======================\n[X]==>");
        scanf("%d", choix);  printf("\n");
    } while (*choix<0 || *choix>3);
}
*/

#include <allegro.h>
#include "GRAPHISMES/graphismes.h"

void menu(int *choix) {
    BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
    int selected = 0;
    const int nb_options = 4;
    const char *options[] = {
        "0) Partie locale",
        "1) Créer une LAN",
        "2) Rejoindre une LAN",
        "3) Quitter"
    };

    while (!key[KEY_ENTER]) {
        clear(buffer);

        textout_centre_ex(buffer, font, "======= MENU =======", SCREEN_W / 2, 100, makecol(255, 255, 255), -1);

        for (int i = 0; i < nb_options; i++) {
            int y = 160 + i * 40;
            int color = (i == selected) ? makecol(255, 255, 0) : makecol(255, 255, 255);
            textout_centre_ex(buffer, font, options[i], SCREEN_W / 2, y, color, -1);
        }

        textout_centre_ex(buffer, font, "Utilisez les flèches ↑ ↓ pour naviguer et Entrée pour valider", SCREEN_W / 2, 340, makecol(150, 150, 150), -1);

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        if (key[KEY_UP]) {
            selected = (selected - 1 + nb_options) % nb_options;
            rest(150); // Anti-rebond
        }
        if (key[KEY_DOWN]) {
            selected = (selected + 1) % nb_options;
            rest(150); // Anti-rebond
        }
    }

    *choix = selected;

    destroy_bitmap(buffer);
    clear_keybuf();
}
