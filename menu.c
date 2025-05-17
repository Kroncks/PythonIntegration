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
    curseur = curseur_redimensionne;

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

    // Charger le curseur
    curseur = load_bitmap("../Projet/Graphismes/Interface/Curseur/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }
    appliquer_transparence_curseur(curseur);

    // Redimensionner le curseur à 32×32
    BITMAP* curseur_redimensionne = create_bitmap(32, 32);
    if (!curseur_redimensionne) {
        allegro_message("Erreur lors de la création du curseur redimensionné !");
        exit(EXIT_FAILURE);
    }
    stretch_blit(curseur, curseur_redimensionne,
                 0, 0, curseur->w, curseur->h,
                 0, 0, 32, 32);
    curseur = curseur_redimensionne;

    // Charger l'image du numéro de joueur (1 à 4)
    char chemin_joueur[256];
    sprintf(chemin_joueur,
            "../Projet/Graphismes/Menus/Joueurs/%d.bmp",
            num + 1);
    BITMAP* img_joueur = charger_et_traiter_image(
        chemin_joueur, 256, 64);
    if (!img_joueur) {
        allegro_message("Erreur lors du chargement de l'image du numéro de joueur !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image de la case de saisie du pseudo
    BITMAP* casePseudo = charger_et_traiter_image(
        "../Projet/Graphismes/Interface/Pseudo/casePseudo.bmp",
        1235, 128);
    if (!casePseudo) {
        allegro_message("Erreur lors du chargement de la case pseudo !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image décorative « Choisis ton pseudo »
    BITMAP* choixPseudo = charger_et_traiter_image(
        "../Projet/Graphismes/Menus/ChoisisPseudo/chooseYourPseudo.bmp",
        256, 128);
    if (!choixPseudo) {
        allegro_message("Erreur lors du chargement de l'image décorative !");
        exit(EXIT_FAILURE);
    }
    appliquer_transparence_curseur(choixPseudo);

    // Initialisation du bouton « Valider »
    const char* bouton_paths[] = {
        "../Projet/Graphismes/Menus/Boutons/VALIDATE.bmp"
    };
    const int nb_boutons = 1;
    Bouton boutons[nb_boutons];
    init_boutons(boutons, bouton_paths, nb_boutons);
    int marge = 30;
    boutons[0].rect.x = SCREEN_W - boutons[0].rect.w - marge;
    boutons[0].rect.y = SCREEN_H - boutons[0].rect.h - marge;

    show_mouse(NULL);

    // Préparation de la structure Perso
    Perso self;
    self.x = self.y = -1;
    strcpy(self.pseudo, "");
    strcpy(self.avatar, "a");

    // Boucle principale d'interaction
    while (1) {
        // Effacer le buffer
        clear_to_color(buffer, makecol(0, 0, 0));

        // Dessiner le fond
        stretch_blit(fond, buffer,
                     0, 0, fond->w, fond->h,
                     0, 0, SCREEN_W, SCREEN_H);

        // Afficher le bouton « Valider »
        afficher_boutons(buffer, boutons, nb_boutons);

        // Afficher l'image du numéro de joueur
        draw_sprite(buffer, img_joueur,
                    SCREEN_W/100 + 450,
                    SCREEN_H/100 + 150);

        // Détecter clic sur « Valider »
        if (mouse_b & 1) {
            int index = bouton_clique(boutons, nb_boutons, mouse_x, mouse_y);
            if (index != -1 && strlen(self.pseudo) > 0)
                break;
        }

        // Définition de la zone de pseudo
        int zone_x = SCREEN_W / 10;
        int zone_y = SCREEN_H / 5;
        int zone_w = SCREEN_W / 2;
        int zone_h = SCREEN_H / 12;

        // Afficher la case pseudo
        draw_sprite(buffer, casePseudo, zone_x, zone_y);

        // Afficher l'image décorative à gauche de la case
        int deco_x = zone_x - choixPseudo->w + 80;
        int deco_y = zone_y + (casePseudo->h - choixPseudo->h) / 2 - 200;
        draw_sprite(buffer, choixPseudo, deco_x, deco_y);

        // Afficher le texte du pseudo entré
        char pseudo_affiche[22];
        sprintf(pseudo_affiche, "%s", self.pseudo);
        BITMAP* texte_pseudo = create_bitmap(8 * 20, 16);
        clear_to_color(texte_pseudo, makecol(255, 0, 255));
        textprintf_ex(texte_pseudo, font, 0, 0,
                      makecol(0, 0, 0), -1,
                      "%s", pseudo_affiche);
        stretch_sprite(buffer, texte_pseudo,
                       zone_x + 130, zone_y + 47,
                       zone_w - 20, zone_h - 15);
        destroy_bitmap(texte_pseudo);

        // Gestion de la saisie clavier
        if (keypressed()) {
            int keycode = readkey();
            int k = keycode >> 8;
            char ch = keycode & 0xFF;
            if (k == KEY_BACKSPACE && strlen(self.pseudo) > 0) {
                self.pseudo[strlen(self.pseudo) - 1] = '\0';
            } else if (k == KEY_DEL) {
                self.pseudo[0] = '\0';
            } else if (isprint(ch) && strlen(self.pseudo) < 20) {
                int len = strlen(self.pseudo);
                self.pseudo[len] = ch;
                self.pseudo[len + 1] = '\0';
            }
        }

        // ** Afficher le curseur personnalisé avec masque **
        stretch_sprite(buffer,
                       curseur_redimensionne,
                       mouse_x, mouse_y,
                       32, 32);

        // Transférer à l'écran
        blit(buffer, screen,
             0, 0, 0, 0,
             SCREEN_W, SCREEN_H);

        rest(10);
    }

    // --- Libération des ressources ---
    destroy_bitmap(buffer);
    destroy_bitmap(fond);
    destroy_bitmap(curseur_redimensionne);
    destroy_bitmap(img_joueur);
    destroy_bitmap(casePseudo);
    destroy_bitmap(choixPseudo);
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
    BITMAP* fond = load_bitmap("../DATA/MENU/wait.bmp", NULL);
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