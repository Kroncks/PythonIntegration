#include "menu.h"
#include <stdio.h>
#include "GRAPHISMES/graphismes.h"
#include <ctype.h>

extern BITMAP* curseur;
extern void afficher_curseur(BITMAP* buffer);

// Fonction pour afficher le curseur à la position de la souris
void afficher_curseur(BITMAP* buffer) {
    // Afficher le curseur personnalisé à la position de la souris
    draw_sprite(buffer, curseur, mouse_x, mouse_y);
}

#include "GRAPHISMES/bouttons.h"


// Fonction du menu
void menu(int* choix) {
    // Créer un buffer pour le double buffering
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image de fond
    BITMAP* background = load_bitmap("../Projet/Graphismes/Menus/Background/1.bmp", NULL);
    if (!background) {
        allegro_message("Impossible de charger l’image de fond !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image du curseur
    curseur = load_bitmap("../Projet/Graphismes/Interface/Curseur/curseur.bmp", NULL);
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
    curseur = curseur_redimensionne;

    // Charger les chemins des boutons
    const char* paths[] = {
        "../Projet/Graphismes/Menus/Boutons/LOCAL.bmp",
        "../Projet/Graphismes/Menus/Boutons/CREATELAN.bmp",
        "../Projet/Graphismes/Menus/Boutons/JOINLAN.bmp",
        "../Projet/Graphismes/Menus/Boutons/EXIT.bmp"
    };
    const int nb_boutons = 4;
    Bouton boutons[nb_boutons];

    // Initialiser les boutons
    init_boutons(boutons, paths, nb_boutons);

    // Cacher le curseur natif et afficher le curseur personnalisé
    show_mouse(NULL);

    // Boucle principale du menu
    while (1) {
        // Effacer le buffer
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

        rest(10);
    }

    // Libérer les ressources
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(curseur_redimensionne);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
}

Perso init_player_graphique(int num) {
    // Créer un buffer pour le double buffering
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image de fond
    BITMAP* fond = load_bitmap("../Projet/Graphismes/Menus/Background/1.bmp", NULL);
    if (!fond) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image du curseur
    curseur = load_bitmap("../Projet/Graphismes/Interface/Curseur/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Dimensions désirées du curseur (ex : 32x32)
    int curseur_largeur = 32;
    int curseur_hauteur = 32;

    // Redimensionner le curseur
    BITMAP* curseur_redimensionne = create_bitmap(curseur_largeur, curseur_hauteur);
    if (!curseur_redimensionne) {
        allegro_message("Erreur lors de la création du curseur redimensionné !");
        exit(EXIT_FAILURE);
    }
    stretch_blit(curseur, curseur_redimensionne, 0, 0, curseur->w, curseur->h, 0, 0, curseur_largeur, curseur_hauteur);
    curseur = curseur_redimensionne;

    // Charger le bouton "Valider"
    const char* bouton_paths[] = { "../Projet/Graphismes/Menus/Boutons/VALIDATE.bmp" };
    const int nb_boutons = 1;
    Bouton boutons[nb_boutons];
    init_boutons(boutons, bouton_paths, nb_boutons);

    int marge = 30;
    Bouton* b = &boutons[0];

    b->rect.x = SCREEN_W - b->rect.w - marge;
    b->rect.y = SCREEN_H - b->rect.h - marge;

    show_mouse(NULL); // Curseur natif caché

    // Initialisation du joueur
    Perso self;
    self.x = self.y = -1;
    strcpy(self.pseudo, "");
    strcpy(self.avatar, "a\0");

    // Boucle d'affichage
    while (1) {
        clear_to_color(buffer, makecol(0, 0, 0));
        stretch_blit(fond, buffer, 0, 0, fond->w, fond->h, 0, 0, SCREEN_W, SCREEN_H);
        afficher_boutons(buffer, boutons, nb_boutons);

        // Clic sur le bouton
        if (mouse_b & 1) {
            int index = bouton_clique(boutons, nb_boutons, mouse_x, mouse_y);
            if (index != -1) if (strlen(self.pseudo)>0) break;
        }

        // Taille cible du texte (proportionnelle à l'écran)
        int largeur = SCREEN_W / 4;   // 25% largeur écran
        int hauteur = SCREEN_H / 10;  // 10% hauteur écran

        // Afficher le numéro du joueur
        char texte_num[32];

        // Créer un bitmap temporaire pour le texte
        BITMAP* texte = create_bitmap(100, 20);  // Taille initiale arbitraire
        clear_to_color(texte, makecol(255, 0, 255)); // fond magenta (transparence)

        // Dessiner le texte dedans
        sprintf(texte_num, "Joueur %d", num+1);
        textprintf_ex(texte, font, 0, 0, makecol(255, 255, 0), -1, texte_num);

        // Rendre le magenta transparent
        set_trans_blender(0, 0, 0, 0);
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        stretch_sprite(buffer, texte, SCREEN_W / 100, SCREEN_H / 100, largeur, hauteur);
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
        destroy_bitmap(texte);

        // --- Affichage zone de pseudo ---
        int zone_x = SCREEN_W / 10;
        int zone_y = SCREEN_H / 5;
        int zone_w = SCREEN_W / 2;
        int zone_h = SCREEN_H / 12;

        // Fond de la zone de texte
        rectfill(buffer, zone_x, zone_y, zone_x + zone_w, zone_y + zone_h, makecol(50, 50, 50));
        rect(buffer, zone_x, zone_y, zone_x + zone_w, zone_y + zone_h, makecol(255, 255, 255));

        // Afficher le texte entré
        char pseudo_affiche[20+ 2];
        sprintf(pseudo_affiche, "%s", self.pseudo);

        // Création d'un BITMAP temporaire pour dessiner le pseudo
        BITMAP* texte_pseudo = create_bitmap(8 * 20, 16);  // Taille en fonction du texte brut
        clear_to_color(texte_pseudo, makecol(255, 0, 255));  // fond transparent

        textprintf_ex(texte_pseudo, font, 0, 0, makecol(255, 255, 255), -1, "%s", pseudo_affiche);

        // Affichage redimensionné dans la zone (on laisse 5px de marge)
        int texte_largeur = zone_w - 20;
        int texte_hauteur = zone_h - 15;
        stretch_sprite(buffer, texte_pseudo, zone_x + 20, zone_y + 20, texte_largeur, texte_hauteur);

        destroy_bitmap(texte_pseudo);

        if (keypressed()) {
            int keycode = readkey();          // Lire une seule fois
            int k = keycode >> 8;             // Code touche spéciale
            char ch = keycode & 0xFF;         // Caractère ASCII

            if (k == KEY_BACKSPACE && strlen(self.pseudo) > 0) {
                self.pseudo[strlen(self.pseudo) - 1] = '\0';
            } else if (k == KEY_DEL) {
                self.pseudo[0] = '\0'; // Tout effacer
            } else if (isprint(ch) && strlen(self.pseudo) < 20) {
                int len = strlen(self.pseudo);
                self.pseudo[len] = ch;
                self.pseudo[len + 1] = '\0';
            }
        }

        // Curseur
        stretch_sprite(buffer, curseur_redimensionne, mouse_x, mouse_y, curseur_largeur, curseur_hauteur);
        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(10);
    }

    destroy_bitmap(buffer);
    destroy_bitmap(fond);
    destroy_bitmap(curseur_redimensionne);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
    return self;
}


void menu_waiting() {
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }
    BITMAP* fond = load_bitmap("../Projet/Graphismes/Menus/Wait/waitPLS.bmp", NULL);
    if (!fond) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }
    stretch_blit(fond, buffer, 0, 0, fond->w, fond->h, 0, 0, SCREEN_W, SCREEN_H);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    destroy_bitmap(buffer);
    destroy_bitmap(fond);
    clear_keybuf();
}