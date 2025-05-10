#include "name.h"
#include "version.h"
#include "LAN/LAN.h"
#include "JEU/jeu.h"

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

    int choix=0;
    while (choix!=3) {
        do {
            printf("\n=======[ MENU ]=======\n 0) Partie locale\n 1) Creer une LAN\n 2) Rejoindre une LAN\n 3) Quitter\n======================\n[X]==>");
            scanf("%d", &choix);  printf("\n");
        } while (choix<0 || choix>3);
        switch (choix) {
            case 0: {
                init_nb_players();
                printf("\nChoix des joueurs\n");
                Perso liste[NB_JOUEURS];
                for (int i = 0; i < NB_JOUEURS; i++) {
                    liste[i]=init_player();
                }
                local(liste);
                break;
            }
            case 1:
                init_nb_players();
                self = init_player();
                serveur();
                attendre_serveur();
                client(username, self);
                break;
            case 2:
                init_nb_players();
                self = init_player();
                client(username, self);
                break;
            default:
                break;
        }
    }
    return 0;
} END_OF_MAIN()

void initialisation_allegro() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(desktop_color_depth());

    if(set_gfx_mode(GFX_AUTODETECT_WINDOWED,800,600,0,0)!=0)
    {
        allegro_message("probleme mode graphique");
        allegro_exit();
        exit(EXIT_FAILURE);
    }
}