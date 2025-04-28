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

    int choix=0;
    while (choix!=3) {
        do {
            printf("\n=======[ MENU ]=======\n 1) Creer une LAN\n 2) Rejoindre une LAN\n 3) Quitter\n======================\n[X]==>");
            scanf("%d", &choix);  printf("\n");fflush(stdin);
        } while (choix<1 || choix>3);
        switch (choix) {
            case 1:
                serveur();
                attendre_serveur();
                client(username);
                break;
            case 2:
                client(username);
                break;
            case 3:
                break;
        }
    }
    return 0;
}
