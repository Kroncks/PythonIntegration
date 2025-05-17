#include "jeu.h"


BITMAP* curseur;  // Déclaration du curseur global

void init_coord(Game * game) {
    game->players[0].x = game->players[0].y = 0;
    game->players[1].x = PLAT_X-1; game->players[1].y = PLAT_Y-1;

    game->players[2].x = 0; game->players[2].y = PLAT_Y-1;
    game->players[3].x = PLAT_X-1; game->players[3].y = 0;
}

void init_plato(Game * game) {
    for (int i=0; i<PLAT_X; i++) { //init plato
        for (int j=0; j<PLAT_Y; j++) {
            game->plateau[i][j] = ((rand() % TILE_COUNT-1) +1 ) * ( rand() % 2) ;
        }
    }
    int rand_x = -(rand() % 2);
    for (int i=0; i<PLAT_X; i++) {
        game->plateau[0][i] = game->plateau[PLAT_Y-1][i] = game->plateau[PLAT_Y/2 + rand_x][i] =0;
    }
    int rand_y = -(rand() % 2);
    for (int i=0; i<PLAT_Y; i++) {
        game->plateau[i][0] = game->plateau[i][PLAT_Y-1] = game->plateau[i][PLAT_Y/2 + rand_y] =0;
    }
}

void transfert_temp_resistance(int r_temp, float* r_resistance) {
    if (r_temp==0) *r_resistance = 0.5;
    else if (r_temp==1) *r_resistance = 1;
    else if (r_temp==2) *r_resistance = 2;
}

void init_classe(t_classe classes_disponibles[12]) {
    int lenght_nom [12][4];
    char filename[100];
    int temp;
    FILE* p_fichier_classe = fopen("../Projet/Fichiers textes/DefClasses.txt", "r");

    //chargement des tailles des noms competences des 12 classes
    for (int i=0; i<12;i++) {
        for (int j=0; j<4;j++) {
            fscanf(p_fichier_classe,"%d", &lenght_nom[i][j]);
        }
    }

    //Chargement des données des 12 classes
    for (int j=0;j<12;j++) {
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].pv);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].mana);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].endurance);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].force);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].dexterite);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].intelligence);
        fscanf(p_fichier_classe,"%d", &classes_disponibles[j].foi);


        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_contandant);
        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_tranchant);
        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_percant);
        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_eau);
        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_feu);
        fscanf(p_fichier_classe,"%d", &temp);
        transfert_temp_resistance(temp, &classes_disponibles[j].r_terre);

        for (int i=0; i<4;i++) {
            char token;
            fscanf(p_fichier_classe, "%c", &token); //Liberation espace
            fgets(classes_disponibles[j].competences[i].nom_competence, lenght_nom[j][i]+1, p_fichier_classe);
            fscanf(p_fichier_classe,"%d", &classes_disponibles[j].competences[i].ID_competence);
            fscanf(p_fichier_classe,"%d", &classes_disponibles[j].competences[i].degat);
            fscanf(p_fichier_classe,"%d", &classes_disponibles[j].competences[i].p_attaque);
            fscanf(p_fichier_classe,"%d", &classes_disponibles[j].competences[i].portee);
            fscanf(p_fichier_classe, "%c", &token); //Liberation espace
            fscanf(p_fichier_classe,"%c", &classes_disponibles[j].competences[i].type_degat);
            fscanf(p_fichier_classe, "%c", &token); //Liberation espace
            fscanf(p_fichier_classe,"%c", &classes_disponibles[j].competences[i].type_stat);
        }
    }
    fclose(p_fichier_classe);

    //Chargement des sprites des 12 classes
    for (int j=0;j<12;j++) {
        for (int i=0; i<8; i++) {
            snprintf(filename, sizeof(filename),"../Projet/Graphismes/Animations/Persos/%d/%d.bmp", j+1,i+1);
            classes_disponibles[j].sprite[i] = charger_et_traiter_image(filename, 64,64);
            if (classes_disponibles[j].sprite[i]==NULL) {
                printf("\nPerso %d sprite %d introuvable", j+1,i+1);
            }
            else {
                printf("\nPerso %d sprite %d chargee", j+1,i+1);
            }
        }
        for (int i=0; i<4;i++) {
            for (int k=0;k<3;k++) {
                snprintf(filename, sizeof(filename),"../Projet/Graphismes/Animations/Competences/%d/%d/%d.bmp", j+1,i+1,k+1);
                classes_disponibles[j].competences[i].sprite[k] = charger_et_traiter_image(filename, 64,64);
            }
        }
    }
}

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


Perso init_player(int num) {
    Perso self;
    self.x = self.y = -1;
    viderBuffer();
    printf("%d\n", num);
    do {
        printf("Pseudo : ");
        fgets(self.pseudo, sizeof(self.pseudo), stdin);
        self.pseudo[strcspn(self.pseudo, "\r\n")] = '\0';

        printf("Avatar : ");
        fgets(self.avatar, sizeof(self.avatar), stdin);
        self.avatar[strcspn(self.avatar, "\r\n")] = '\0';

        printf("[VALIDER]");
    } while (getchar() != '\n');
    printf("\n");
    return self;
}

void tour(Game * game, int num, char * data) {
    printf("[Game] Choisir x\n[X]==>");
    scanf("%d", &game->players[num].x);
    printf("[Game] Choisir y\n[Y]==>");
    scanf("%d", &game->players[num].y);

    if (data != NULL) sprintf(data, "%d %d", game->players[num].x, game->players[num].y);
}

void process_data(Game * game, int num, char * data) {
    sscanf(data, "%d %d", &game->players[num].x, &game->players[num].y);
}

void init_nb_players() {
    FILE *f;

    do {
        printf("Entrez le nombre de joueurs (2 ou 4) : ");
        scanf("%d", &NB_JOUEURS);
    } while (NB_JOUEURS != 2 && NB_JOUEURS != 4);


    f = fopen("../LAN/tmp/NB_PLAYERS.txt", "w");
    if (!f) {
        perror("[INIT] Erreur ouverture NB_PLAYERS.txt");
        exit(EXIT_FAILURE);
    }

    fprintf(f, "%d", NB_JOUEURS);
    fclose(f);

    printf("[INIT] NB_JOUEURS = %d\n", NB_JOUEURS);
}

void init_game(socket_t sock, Game * game, int num, Perso self) {
    long int received;
    char buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    for (int i=0; i<NB_JOUEURS; i++) {
        if (num == i) {
            strcpy(game->players[i].pseudo, self.pseudo);
            strcpy(buffer, game->players[i].pseudo);
            send(sock, buffer, strlen(buffer), 0); // envoi du pseudo
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
            strcpy(game->players[i].pseudo, buffer);
            printf("[GAME] player %d saved : %s\n",i, game->players[i].pseudo );
        }
    }
#ifdef WIN32
    Sleep(0.2); // version windows
#else
    sleep(0.2);
#endif
    printf("log\n");
    for (int i=0; i<NB_JOUEURS; i++) {
        if (num == i) {
            strcpy(game->players[i].avatar, self.avatar);
            printf("sending avatar : %s\n", game->players[i].avatar);
            strcpy(buffer, game->players[i].avatar);
            send(sock, buffer, strlen(buffer), 0); // envoi de l'avatar
            printf("data sent");
        } else {
            get_data(sock, &received, buffer, i,  &quit); // on attends de recevoir les données
            strcpy(game->players[i].avatar, buffer);
            printf("[GAME] player %d avatar : %s\n",i, game->players[i].avatar );
        }
    }


    if ( num==0 ) {
        init_plato(game);
        for (int i=0; i<PLAT_X; i++) { //load plato into buffer
            for (int j=0; j<PLAT_Y; j++) {
                buffer[j+i*PLAT_X] =  game->plateau[i][j]+48;
            }
        }
        // envoyer le plateau
#ifdef WIN32
        Sleep(0.2); // version windows
#else
        sleep(0.2);
#endif
        buffer[PLAT_X*PLAT_Y] = '\0';
        send(sock, buffer, strlen(buffer), 0);
        printf("[GAME] buffer : %s\n",buffer);
        // envoyer le theme
#ifdef WIN32
        Sleep(0.2); // version windows
#else
        sleep(0.2);
#endif
        buffer[0] = game->theme + 'a';
        buffer[1] = '\0';
        send(sock, buffer, strlen(buffer), 0);
        printf("[GAME] buffer : %s\n",buffer);
    }else {
        printf("[GAME] waiting for the map\n");
        //recevoir le plateau
        get_data(sock, &received, buffer, 0, &quit);
        printf("[GAME] received data : %s\n",buffer);
        for (int i=0; i<PLAT_X; i++) {
            for (int j=0; j<PLAT_Y; j++) {
                game->plateau[i][j] = buffer[j+i*PLAT_X]-48;
            }
        }
        printf("[GAME] waiting for the theme\n");
        //recevoir le theme
        get_data(sock, &received, buffer, 0, &quit);
        printf("[GAME] received data : %s\n",buffer);
        game->theme = buffer[0] - 'a';
    }
    //coordonées de base des joueurs :
    init_coord(game);
}


void show(Game game, int n_turns, int num) {
    printf("====================[%d]====================\n", n_turns);
    for (int i=0; i<NB_JOUEURS; i++) {
        printf("%d) %s (%d,%d)\n", i, game.players[i].pseudo, game.players[i].x, game.players[i].y);
    }
    printf("\n>[%d][%s]\navatar : %s\n\n",num,game.players[num].pseudo, game.players[num].avatar);
    int flag = 0;
    for (int i = 0; i < PLAT_Y; i++) {
        for (int j = 0; j < PLAT_X; j++) {
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

void show_graphique(Game game,int n_turns,int i, BITMAP* buffer, BITMAP* curseur) {
    // Affiche le fond
    if (game.map.background) {
        stretch_blit(game.map.background, buffer,
                     0, 0, game.map.background->w, game.map.background->h,
                     0, 0, SCREEN_W, SCREEN_H);
    }
    //====

    int tile_width = 64;
    int tile_height = 40;
    int origin_x = SCREEN_W / 2;  // Utilise SCREEN_W et SCREEN_H
    int offset_y = SCREEN_H / 2 - tile_height  * 10;

    for (int y = 0; y < PLAT_Y; y++) {
        for (int x = 0; x < PLAT_X; x++) {
            int id = game.plateau[y][x];

            if (id >= 0 && id < TILE_COUNT && game.map.images[id]) {
                int iso_x = (x - y) * (tile_width / 2) + origin_x;
                int iso_y = (x + y) * (tile_height / 2) + offset_y;

                // Ne dessine que si les coordonnées sont dans l'écran
                if (iso_x + tile_width > 0 && iso_x < SCREEN_W &&
                    iso_y + tile_height > 0 && iso_y < SCREEN_H) {
                    draw_sprite(buffer, game.map.images[id], iso_x, iso_y);
                    }
            }
        }
    }



    //====
    stretch_sprite(buffer, curseur, mouse_x, mouse_y, 32, 32);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

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


void jouer_graphique(socket_t sock, Game * game, int num) {
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }
    game->map.background = load_bitmap("../DATA/GAME/MAP/BACKGROUND/2.bmp", NULL);
    if (!game->map.background) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < TILE_COUNT; i++) {
        char path[100];
        sprintf(path, "../DATA/GAME/MAP/TUILES/2/%d.bmp", i+1);
        game->map.images[i] = charger_et_traiter_image(path, 64, 64);
    }





    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Dimensions désirées du curseur (ex : 32x32)


    // Redimensionner le curseur
    BITMAP* curseur_redimensionne = create_bitmap(32, 32);
    if (!curseur_redimensionne) {
        allegro_message("Erreur lors de la création du curseur redimensionné !");
        exit(EXIT_FAILURE);
    }
    stretch_blit(curseur, curseur_redimensionne, 0, 0, curseur->w, curseur->h, 0, 0, 32, 32);
    curseur = curseur_redimensionne;



    // ===
    long int received;
    char LAN_buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    int n_turns = 0;
    int next = 0;
    show(*game, 0, num); // log
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            if (num == i) {
                while (!next) {
                    show_graphique(*game,n_turns,i, buffer, curseur); // affiche l'ecrant de jeu
                    //tour_graphique(game, i, &next ); // verifie les actions du joueur et joue joue
                    rest(10);
                }
                tour(game, i, LAN_buffer); // le joueur joue
                send(sock, LAN_buffer, strlen(LAN_buffer), 0); // les données sont envoyées
                printf("[Game] Data sent\n");
            } else {
                show_graphique(*game,n_turns,i, buffer, curseur); // affiche l'ecrant de jeu
                get_data(sock, &received, LAN_buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, LAN_buffer); // on traite les données des autres joueurs
                show_graphique(*game,n_turns,i, buffer, curseur); // affiche l'ecrant de jeu
            }
        }
    }
    destroy_bitmap(buffer);
    destroy_bitmap(game->map.background);
    clear_keybuf();
}

// ---- local


void jouer_local(Game * game) {
    int quit = 0;
    int n_turns = 0;
    show(*game, 0, 0);
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            tour(game, i, NULL ); // le joueur joue
            show(*game,n_turns,i);
        }
    }
}





void init_local_game(Game * game, Perso * liste) {
    for (int i=0; i<NB_JOUEURS; i++) {
        game->players[i]=liste[i];
    }
        init_plato(game);
    init_coord(game);

}



void jouer_local_graphique(Game * game) {
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    game->map.background = load_bitmap("../DATA/GAME/MAP/BACKGROUND/2.bmp", NULL);
    if (!game->map.background) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < TILE_COUNT; i++) {
        char path[100];
        sprintf(path, "../DATA/GAME/MAP/TUILES/2/%d.bmp", i+1);
        game->map.images[i] = charger_et_traiter_image(path, 64, 64);
    }


    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Dimensions désirées du curseur (ex : 32x32)


    // Redimensionner le curseur
    BITMAP* curseur_redimensionne = create_bitmap(32, 32);
    if (!curseur_redimensionne) {
        allegro_message("Erreur lors de la création du curseur redimensionné !");
        exit(EXIT_FAILURE);
    }
    stretch_blit(curseur, curseur_redimensionne, 0, 0, curseur->w, curseur->h, 0, 0, 32, 32);
    curseur = curseur_redimensionne;
    int quit = 0;
    int next = 0;
    int n_turns = 0;
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            while (!next) {
                show_graphique(*game,n_turns,i, buffer, curseur); // affiche l'ecrant de jeu
                //tour_graphique(game, i, &next ); // verifie les actions du joueur et joue joue
                rest(10);
            }
            //check_victory(game, &quit);
        }
    }
    destroy_bitmap(buffer);
    destroy_bitmap(game->map.background);
    clear_keybuf();
}


// Initialisation et affichage du menu de sélection du nombre de joueurs
void init_nb_players_graphique() {
    Sleep(300);
    printf("log init\n");
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

    // Charger les boutons pour 2 et 4 joueurs
    const char* paths[] = {
        "../DATA/MENU/BOUTTONS/2_players.bmp",
        "../DATA/MENU/BOUTTONS/4_players.bmp"
    };
    const int nb_boutons = 2;
    Bouton boutons[nb_boutons];

    // Initialiser les boutons
    init_boutons(boutons, paths, nb_boutons);

    // Cacher le curseur natif et afficher le curseur personnalisé
    show_mouse(NULL);

    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Dimensions du curseur (taille fixe pour tous les menus)
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

    // Libérer l'ancienne image du curseur (si ce n'est plus nécessaire)
    destroy_bitmap(curseur);

    // Boucle principale du menu pour choisir le nombre de joueurs
    while (1) {
        // Effacer le buffer (mettre à zéro)
        clear_to_color(buffer, makecol(0, 0, 0));

        // Afficher l'arrière-plan dans le buffer
        stretch_blit(background, buffer, 0, 0, background->w, background->h, 0, 0, SCREEN_W, SCREEN_H);

        // Afficher les boutons dans le buffer
        afficher_boutons(buffer, boutons, nb_boutons);

        // Détecter les clics sur les boutons
        if (mouse_b & 1) {  // clic gauche
            int index = bouton_clique(boutons, nb_boutons, mouse_x, mouse_y);
            if (index != -1) {
                // Sélectionner le nombre de joueurs en fonction du bouton cliqué
                if (index == 0) {
                    NB_JOUEURS = 2;
                } else if (index == 1) {
                    NB_JOUEURS = 4;
                }

                // Sauvegarder le nombre de joueurs dans le fichier
                FILE *f = fopen("../LAN/tmp/NB_PLAYERS.txt", "w");
                if (!f) {
                    allegro_message("[INIT] Erreur ouverture NB_PLAYERS.txt");
                    exit(EXIT_FAILURE);
                }
                fprintf(f, "%d", NB_JOUEURS);
                fclose(f);

                printf("[INIT] NB_JOUEURS = %d\n", NB_JOUEURS);
                break;  // Sortir de la boucle après la sélection
            }
        }

        // Afficher le curseur redimensionné dans le buffer
        stretch_sprite(buffer, curseur_redimensionne, mouse_x, mouse_y, curseur_largeur, curseur_hauteur);

        // Transférer le contenu du buffer à l'écran
        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        // Petite pause pour éviter la surcharge CPU
        rest(10);
    }

    // Libérer les ressources
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(curseur_redimensionne);
    detruire_boutons(boutons, nb_boutons);
    clear_keybuf();
}
