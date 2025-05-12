#include "menu.h"
#include <stdio.h>
#include "GRAPHISMES/graphismes.h"

#include "globals.h"
BITMAP* curseur;

void afficher_curseur(BITMAP* buffer) {
    // Afficher le curseur personnalisé à la position de la souris
    draw_sprite(buffer, curseur, mouse_x, mouse_y);
}

#include "GRAPHISMES/bouttons.h"

void menu(int* choix) {
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    BITMAP* background = load_bitmap("../DATA/MENU/1.bmp", NULL);
    if (!background) {
        allegro_message("Impossible de charger l’image de fond !");
        exit(EXIT_FAILURE);
    }

    const char* paths[] = {
        "../DATA/MENU/BOUTTONS/1.bmp",
        "../DATA/MENU/BOUTTONS/2.bmp",
        "../DATA/MENU/BOUTTONS/3.bmp",
        "../DATA/MENU/BOUTTONS/4.bmp"
    };
    const int nb_boutons = 4;
    Bouton boutons[nb_boutons];
    init_boutons(boutons, paths, nb_boutons);

    show_mouse(screen);
    while (1==1) {
        stretch_blit(background, buffer, 0, 0, background->w, background->h, 0, 0, SCREEN_W, SCREEN_H);
        afficher_boutons(buffer, boutons, nb_boutons);
        afficher_curseur(buffer);
        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        if (mouse_b & 1) {  // clic gauche
            int index = bouton_clique(boutons, nb_boutons, mouse_x, mouse_y);
            if (index != -1) {
                *choix = index;
                break;
            }
        }

        rest(10);
    }

    destroy_bitmap(buffer);
    destroy_bitmap(background);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
}

