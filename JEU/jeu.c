#include "jeu.h"

//void selectPerso()

void tour(Game * game, int num, char * data) {
    printf("[Game] Choisir x\n[X]==>");
    scanf("%d", &game->players[num].x);
    printf("[Game] Choisir y\n[Y]==>");
    scanf("%d", &game->players[num].y);

    sprintf(data, "%d %d", game->players[num].x, game->players[num].y);
}

void process_data(Game * game, int num, char * data) {
    sscanf(data, "%d %d", &game->players[num].x, &game->players[num].y);
}

void init_nb_players() {
    FILE *f = fopen("../LAN/tmp/NB_PLAYERS.txt", "r");
    if (!f) {
        perror("[INIT] Erreur lors de l'ouverture du fichier NB_JOUEURS.txt");
        printf("[INIT] NB_JOUEURS = 4 (défault)");
        NB_JOUEURS=4;
    }
    if (fscanf(f, "%d", &NB_JOUEURS) != 1 || NB_JOUEURS <= 0) {
        fprintf(stderr, "Erreur lecture\n");
        printf("[INIT] NB_JOUEURS = 4 (défault)");
    } else {
        printf("[INIT] NB_JOUEURS = %d\n",NB_JOUEURS);
    }
    fclose(f);
}

void init_game(socket_t sock, Game * game, int num, char * username) {
    init_nb_players();
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    for (int i=0; i<NB_JOUEURS; i++) {
        if (num == i) {
            strcpy(game->players[i].name, username);
            strcpy(buffer, username);
            send(sock, buffer, strlen(buffer), 0); // envoi du pseudo
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
            strcpy(game->players[i].name, buffer);
            printf("[GAME] player %d saved : %s\n",i, game->players[i].name );
        }
    }
    for (int i=0; i<NB_JOUEURS; i++) {
        if (num == i) {
            strcpy(buffer, game->players[i].avatar);
            send(sock, buffer, strlen(buffer), 0); // envoi de l'avatar
            printf("data sent");
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
            strcpy(game->players[i].avatar, buffer);
            printf("[GAME] player %d avatar : %s\n",i, game->players[i].name );
        }
    }


    if ( num==0 ) {
        for (int i=0; i<X; i++) { //init plato
            for (int j=0; j<Y; j++) {
                game->plateau[i][j] = rand() % 10;
                buffer[j+i*X] =  game->plateau[i][j]+48;
            }
        }
        // envoyer le plateau
#ifdef WIN32
        _sleep(0.2); // version windows
#else
        sleep(0.2);
#endif
        buffer[X*Y] = '\0';
        send(sock, buffer, strlen(buffer), 0);
        printf("[GAME] buffer : %s\n",buffer);
    }else {
        printf("[GAME] waiting for the map\n");
        //recevoir le plateau
        get_data(sock, &received, buffer, 0, &quit);
        printf("[GAME] received data : %s\n",buffer);
        for (int i=0; i<X; i++) {
            for (int j=0; j<Y; j++) {
                game->plateau[i][j] = buffer[j+i*X]-48;
            }
        }
    }
    //coordonées de base des joueurs :
    game->players[0].x = 0; game->players[0].y = Y/2;
    game->players[1].x = X-1; game->players[1].y = Y/2;

    game->players[2].x = X/2; game->players[2].y = 0;
    game->players[3].x = X/2; game->players[3].y = Y-1;
}


void show(Game game, int n_turns, int num) {
    printf("====================[%d]====================\n", n_turns);
    for (int i=0; i<NB_JOUEURS; i++) {
        printf("%d) %s (%d,%d)\n", i, game.players[i].name, game.players[i].x, game.players[i].y);
    }
    printf("\n>[%d][%s]\n\n",num,game.players[num].name);
    int flag = 0;
    for (int i = 0; i < Y; i++) {
        for (int j = 0; j < X; j++) {
            if (game.players[num].x == j && game.players[num].y == i) {
                printf("[O] ");
            }else {
                for (int k=0; k<NB_JOUEURS; k++) {
                    if (game.players[k].x == j && game.players[k].y == i) {
                        printf("[X] ");
                        flag = 1;
                        break;
                    }
                }
                if (!flag) printf(" %d  ", game.plateau[i][j]);
                flag = 0;
            }
        }
        printf("\n");
    }
    printf("===========================================\n");
}


void jouer(socket_t sock, Game * game, int num) {
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    int n_turns = 0;
    show(*game, 0, num);
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            if (num == i) {
                tour(game, i, buffer); // le joueur joue
                send(sock, buffer, strlen(buffer), 0); // les données sont envoyées
                printf("[Game] Data sent\n");
                show(*game,n_turns,num);
            } else {
                get_data(sock, &received, buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, buffer); // on traite les données des autres joueurs
                show(*game,n_turns,num);
            }
        }
    }
}