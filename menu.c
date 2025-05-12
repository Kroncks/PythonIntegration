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
    // Créer un buffer pour le double buffering
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image de fond
    BITMAP* background = load_bitmap("../DATA/MENU/1.bmp", NULL);
    if (!background) {
        allegro_message("Impossible de charger l’image de fond !");
        exit(EXIT_FAILURE);
    }

    // Charger les chemins des boutons
    const char* paths[] = {
        "../DATA/MENU/BOUTTONS/1.bmp",
        "../DATA/MENU/BOUTTONS/2.bmp",
        "../DATA/MENU/BOUTTONS/3.bmp",
        "../DATA/MENU/BOUTTONS/4.bmp"
    };
    const int nb_boutons = 4;
    Bouton boutons[nb_boutons];

    // Initialiser les boutons
    init_boutons(boutons, paths, nb_boutons);

    // Cacher le curseur natif et afficher le curseur personnalisé
    show_mouse(NULL);

    // Boucle principale du menu
    while (1) {
        // Effacer le buffer (mettre à zéro)
        clear_to_color(buffer, makecol(0, 0, 0));

        // Afficher l'arrière-plan dans le buffer
        stretch_blit(background, buffer, 0, 0, background->w, background->h, 0, 0, SCREEN_W, SCREEN_H);

        // Afficher les boutons dans le buffer
        afficher_boutons(buffer, boutons, nb_boutons);

        // Détecter les clics sur les boutons
        if (mouse_b & 1) {  // clic gauche
            int index = bouton_clique(boutons, nb_boutons, mouse_x, mouse_y);
            if (index != -1) {
                *choix = index;
                break;
            }
        }

        // Afficher le curseur personnalisé dans le buffer
        masked_blit(curseur, buffer, 0, 0, mouse_x, mouse_y, curseur->w, curseur->h);

        // Transférer le contenu du buffer à l'écran
        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        // Petite pause pour éviter la surcharge CPU
        rest(10);
    }

    // Libérer les ressources
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
}


