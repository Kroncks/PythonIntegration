#include "version.h"

#define LEN 20
#define VERSION_FILE "../version.txt"
#define NB_PLAYERS_FILE "../LAN/tmp/NB_PLAYERS.txt"

#define PRGRM_NAME "CLM"

void n_players() {
    char nb_players[LEN];
    FILE *file = fopen(NB_PLAYERS_FILE, "r");
    if (fgets(nb_players, LEN, file) != NULL) {
        // Enlève le saut de ligne s'il y en a un
        nb_players[strcspn(nb_players, "\n")] = '\0';
        printf(" p%s\n", nb_players);
    } else {
        printf("\nErreur lors de la lecture du nombre de joueurs.\n");
    }
    fclose(file);
}

void version() {
    char version_n[LEN];
    FILE *file = fopen(VERSION_FILE, "r");
    if (fgets(version_n, LEN, file) != NULL) {
        // Enlève le saut de ligne s'il y en a un
        version_n[strcspn(version_n, "\n")] = '\0';
        printf("%s %s",PRGRM_NAME, version_n);
        n_players();
    } else {
        printf("Erreur lors de la lecture de la version.\n");
    }
    fclose(file);
}
