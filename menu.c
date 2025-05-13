#include "menu.h"
#include <stdio.h>
#include "GRAPHISMES/graphismes.h"
#include <ctype.h>

BITMAP* curseur;  // Déclaration du curseur global

// Fonction pour afficher le curseur à la position de la souris
void afficher_curseur(BITMAP* buffer) {
    // Afficher le curseur personnalisé à la position de la souris
    draw_sprite(buffer, curseur, mouse_x, mouse_y);
}

#include "GRAPHISMES/bouttons.h"



// Dans menu.c ou graphismes.c
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

    // Dimensions désirées du curseur (par exemple, 32x32)
    int curseur_largeur = 32;
    int curseur_hauteur = 32;

    // Créer un nouveau bitmap pour stocker le curseur redimensionné
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






Perso init_player_graphique() {
    printf("log\n");
    Perso self;
    self.x = self.y = -1;
    self.pseudo[0] = '\0';
    self.avatar[0] = '\0';
    self.avatar[1] = '\n';
    self.avatar[2] = '\0';

    BITMAP *page = create_bitmap(SCREEN_W, SCREEN_H);
    int selected_avatar = -1;
    char input[16] = "";
    int input_len = 0;

    // Curseur personnalisé
    BITMAP *cursor = load_bitmap("cursor.bmp", NULL);
    show_mouse(NULL);

    while (!key[KEY_ESC]) {
        clear_to_color(page, makecol(0, 0, 0));

        // Mise à jour du texte pseudo
        textprintf_ex(page, font, 20, 20, makecol(255, 255, 255), -1, "Pseudo : %s", input);

        // Boîte de saisie
        rect(page, 120, 15, 120 + 200, 35, makecol(255, 255, 255));

        // Grille d'avatars
        int grid_x = SCREEN_W - 240;
        int grid_y = 20;
        int case_size = 50;
        for (int i = 0; i < 12; i++) {
            int row = i / 3;
            int col = i % 3;
            int x = grid_x + col * (case_size + 10);
            int y = grid_y + row * (case_size + 10);
            rect(page, x, y, x + case_size, y + case_size, makecol(255, 255, 255));
            char c = 'a' + i;
            textprintf_centre_ex(page, font, x + case_size/2, y + case_size/2 - 4, makecol(255, 255, 255), -1, "%c", c);
            if (mouse_b & 1 &&
                mouse_x >= x && mouse_x <= x + case_size &&
                mouse_y >= y && mouse_y <= y + case_size) {
                selected_avatar = i;
                self.avatar[0] = c;
            }
            if (selected_avatar == i) {
                rectfill(page, x + 2, y + 2, x + case_size - 2, y + case_size - 2, makecol(0, 255, 0));
                textprintf_centre_ex(page, font, x + case_size/2, y + case_size/2 - 4, makecol(0, 0, 0), -1, "%c", c);
            }
        }

        // Bouton Valider
        int btn_w = 100, btn_h = 40;
        int btn_x = SCREEN_W - btn_w - 20;
        int btn_y = SCREEN_H - btn_h - 20;
        rect(page, btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, makecol(255, 255, 255));
        textprintf_centre_ex(page, font, btn_x + btn_w / 2, btn_y + 10, makecol(255, 255, 255), -1, "VALIDER");

        // Clic sur bouton
        if (mouse_b & 1 &&
            mouse_x >= btn_x && mouse_x <= btn_x + btn_w &&
            mouse_y >= btn_y && mouse_y <= btn_y + btn_h &&
            strlen(input) > 0 && selected_avatar >= 0) {
            strcpy(self.pseudo, input);
            destroy_bitmap(page);
            destroy_bitmap(cursor);
            return self;
        }

        // Entrée clavier (texte pseudo)
        for (int c = 0; c < KEY_MAX; c++) {
            if (key[c]) {
                if (input_len < 15 && c >= KEY_A && c <= KEY_Z) {
                    input[input_len++] = 'a' + (c - KEY_A);
                    input[input_len] = '\0';
                    rest(150); // anti-rebond
                }
                if (key[KEY_SPACE] && input_len < 15) {
                    input[input_len++] = ' ';
                    input[input_len] = '\0';
                    rest(150);
                }
                if (key[KEY_BACKSPACE] && input_len > 0) {
                    input[--input_len] = '\0';
                    rest(150);
                }
            }
        }

        // Curseur personnalisé
        draw_sprite(page, cursor, mouse_x, mouse_y);
        blit(page, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    }

    destroy_bitmap(page);
    destroy_bitmap(cursor);
    return self;
}