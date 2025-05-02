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
            scanf("%d", &choix);  printf("\n");fflush(stdin);
        } while (choix<1 || choix>3);
        switch (choix) {
            case 0:
                // il faudrait pouvoir connaitre le nombre de joueur en ammont
                Perso liste[4];
                for (int i = 0; i < 4; i++) {}
                //locale();
                break;
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
            case 3:
                break;
        }
    }
    return 0;
}
