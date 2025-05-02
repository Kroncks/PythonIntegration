#include "name.h"
#include "version.h"
#include "LAN/LAN.h"

int main() {
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
        } while (choix<1 || choix>3);
        switch (choix) {
            case 0: {
                // il faudrait pouvoir connaitre le nombre de joueur en ammont
                Perso liste[NB_JOUEURS];
                for (int i = 0; i < NB_JOUEURS; i++) {
                    liste[i]=init_player();
                }
                //locale();
                break;
            }
            case 1:
                self = init_player();
                serveur();
                attendre_serveur();
                client(username, self);
                break;
            case 2:
                self = init_player();
                client(username, self);
                break;
            default:
                break;
        }
    }
    return 0;
}
