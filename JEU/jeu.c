#include "jeu.h"


void tour(Game * game, int num, char * data) {

}

void process_data(Game * game, int num, char * data) {

}

void init_game(socket_t sock, Game * game, int num, char * username) {
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    for (int i=0; i<MAX_JOUEURS; i++) {
        if (num == i) {
            strcpy(game->players[i].name, username);
            strcpy(buffer, username);
            send(sock, buffer, strlen(buffer), 0); // envoi du pseudo
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
            strcpy(game->players[i].name, buffer);
            if (sscanf(buffer, "%*d : %19[^\n]", game->players[i].name) == 1)
                printf("[GAME] player %d saved : %s\n",i, game->players[i].name );
            else
                printf("[GAME] error saving player %d",i);
        }
    }
    if ( num==0 ) {
        for (int i=0; i<X; i++) { //init plato
            for (int j=0; j<Y; j++) {
                game->plateau[i][j] = 0;
            }
        }
        // envoyer le plateau
    }else {
        //recevoir le plateau
    }
}


void show(Game game, int num) {
    printf("==========[%d]==========\n", num);
    for (int i=0; i<MAX_JOUEURS; i++) {
        printf("%d) %s\n", i, game.players[i].name);
    }
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            printf(" %d  ", game.plateau[i][j]);
        }
        printf("\n");
    }
    printf("========================\n");
}


void jouer(socket_t sock, Game * game, int num) {
    show(*game, num);
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;

    while (!quit) {
        for (int i=0; i<MAX_JOUEURS; i++) {
            if (num == i) {
                tour(game, i, buffer); // le joueur joue
                show(*game, i);
                send(sock, buffer, strlen(buffer), 0); // les données sont envoyées
            } else {
                get_data(sock, &received, buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, buffer); // on traite les données des autres joueurs
                show(*game, i);
            }
        }
    }
}