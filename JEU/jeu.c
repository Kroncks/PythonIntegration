#include "jeu.h"


void tour(Game game, int num, char * data) {

}

void process_data(Game game, int num, char * data) {

}

void init_game(socket_t sock, Game game, int num, char * username) {
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    for (int i=0; i<3; i++) {
        if (num == i) {
            strcpy(buffer, username);
            send(sock, buffer, strlen(buffer), 0); // envoi du pseudo
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
        }
    }
    if ( num==0 ) {
        for (int i=0; i<X; i++) { //init plato

        }
        // envoyer le plateau
    }else {
        //recevoir le plateau
    }
}


void show(Game game, int num) {
    printf("==========[%d]==========\n", num);
    printf(">%s\n", game.players[num].name);
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            printf("%d", game.plateau[i][j]);
        }
    }
    printf("========================\n");
}


void jouer(socket_t sock, Game game, int num) {
    show(game, num);
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;

    while (!quit) {
        for (int i=0; i<3; i++) {
            if (num == i) {
                tour(game, i, buffer); // le joueur joue
                show(game, i);
                send(sock, buffer, strlen(buffer), 0); // les données sont envoyées
            } else {
                get_data(sock, &received, buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, buffer); // on traite les données des autres joueurs
                show(game, i);
            }
        }
    }
}