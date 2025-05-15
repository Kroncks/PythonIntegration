#include "menu.h"
#include "name.h"
#include "version.h"
#include "LAN/LAN.h"
#include "JEU/jeu.h"

#include "globals.h"
int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;

BITMAP* curseur;


void initialisation_allegro();

int main() {
    initialisation_allegro();
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    char username[MAX_USERNAME];
    printf("\n");
    version();
    printf("\n");
    name(username);
    Perso self;

    int choix = 0;
    while (choix != 3) {
        menu(&choix);
        switch (choix) {
            case 0: {
                init_nb_players_graphique();
                printf("\nChoix des joueurs (%d)\n", NB_JOUEURS);
                Perso liste[NB_JOUEURS];
                for (int i = 0; i < NB_JOUEURS; i++) {
                    liste[i] = init_player_graphique(i);
                }
                local(liste);
                break;
            }
            case 1:
                init_nb_players_graphique();
                self = init_player_graphique(-1);
                serveur();
                attendre_serveur();
                menu_waiting();
                client(username, self);
                break;
            case 2:
                init_nb_players_graphique();
                self = init_player_graphique(-1);
                menu_waiting();
                client(username, self);
                break;
            default:
                break;
        }
    }

    return 0;
}
END_OF_MAIN()


void redimensionner_curseur(BITMAP* curseur, int target_width, int target_height) {
    BITMAP* curseur_temp = create_bitmap(target_width, target_height);
    stretch_blit(curseur, curseur_temp, 0, 0, curseur->w, curseur->h, 0, 0, target_width, target_height);
    destroy_bitmap(curseur);
    curseur = curseur_temp;  // On remplace l'ancien curseur redimensionné
}


void initialisation_allegro() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(desktop_color_depth());

    get_desktop_resolution(&SCREEN_WIDTH, &SCREEN_HEIGHT);
    printf("resolution : %d, %d", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0) != 0) {
        allegro_message("Problème de mode graphique !");
        allegro_exit();
        exit(EXIT_FAILURE);
    }

    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Erreur chargement du curseur !");
        exit(EXIT_FAILURE);
    }

    BITMAP* temp = create_bitmap(32, 32);
    if (!temp) {
        allegro_message("Erreur création du bitmap redimensionné !");
        exit(EXIT_FAILURE);
    }

    stretch_blit(curseur, temp, 0, 0, curseur->w, curseur->h, 0, 0, 32, 32);
    destroy_bitmap(curseur);
    curseur = temp;

    show_mouse(NULL);  // Désactive le curseur natif
}
