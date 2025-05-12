#include "menu.h"
#include <stdio.h>
#include "GRAPHISMES/graphismes.h"

#include "globals.h"
BITMAP* curseur;

// Fonction pour afficher le curseur à la position de la souris
void afficher_curseur(BITMAP* buffer) {
    // Afficher le curseur personnalisé à la position de la souris
    draw_sprite(buffer, curseur, mouse_x, mouse_y);
}

#define COLOR_TOLERANCE 100

// Fonction pour vérifier si un pixel est de couleur "rose"
int rose(int r, int g, int b) {
    return (abs(r - 255) <= COLOR_TOLERANCE &&
            abs(g - 0)   <= COLOR_TOLERANCE &&
            abs(b - 255) <= COLOR_TOLERANCE);
}

// Appliquer la transparence au curseur
void appliquer_transparence_curseur(BITMAP* curseur) {
    for (int y = 0; y < curseur->h; y++) {
        for (int x = 0; x < curseur->w; x++) {
            int c = getpixel(curseur, x, y);
            int r = getr(c), g = getg(c), b = getb(c);
            if (rose(r, g, b)) {
                putpixel(curseur, x, y, makecol(255, 0, 255));  // Masque transparent
            }
        }
    }
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

    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Redimensionner le curseur pour qu'il soit plus petit
    int curseur_largeur = 32;  // Nouvelle largeur du curseur
    int curseur_hauteur = 32;  // Nouvelle hauteur du curseur

    BITMAP* curseur_redimensionne = create_bitmap(curseur_largeur, curseur_hauteur);
    if (!curseur_redimensionne) {
        allegro_message("Erreur lors de la création du curseur redimensionné !");
        exit(EXIT_FAILURE);
    }

    // Redimensionner l'image du curseur
    stretch_blit(curseur, curseur_redimensionne, 0, 0, curseur->w, curseur->h, 0, 0, curseur_largeur, curseur_hauteur);

    // Libérer l'ancienne image du curseur (si ce n'est plus nécessaire)
    destroy_bitmap(curseur);

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

        // Afficher le curseur redimensionné dans le buffer
        stretch_sprite(buffer, curseur_redimensionne, mouse_x, mouse_y, curseur_largeur, curseur_hauteur);

        // Transférer le contenu du buffer à l'écran
        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        // Petite pause pour éviter la surcharge CPU
        rest(10);
    }

    // Libérer les ressources
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(curseur_redimensionne);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
}
