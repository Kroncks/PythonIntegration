#include "jeu.h"

#include "../globals.h"

#define TILE_HEIGHT 40
#define TILE_WIDTH 64
#define NB_THEMES 4

BITMAP* curseur;  // Déclaration du curseur global
SAMPLE* musique;
BITMAP* liste_avatar[12];
BITMAP* liste_big_avatar[12];
BITMAP* liste_story[12];

void init_coord(Game * game) {
    game->players[0].x = game->players[0].y = 0;
    game->players[1].x = PLAT_X-1; game->players[1].y = PLAT_Y-1;
    game->players[2].x = 0; game->players[2].y = PLAT_Y-1;
    game->players[3].x = PLAT_X-1; game->players[3].y = 0;
    game->theme = rand() % NB_THEMES;
    game->plateau[0][0]= TILE_COUNT;
    game->plateau[PLAT_Y-1][PLAT_X-1]= TILE_COUNT+1;
    if (NB_JOUEURS == 4) {
        game->plateau[PLAT_Y-1][0]= TILE_COUNT+2;
        game->plateau[0][PLAT_X-1]= TILE_COUNT+3;
    }
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


void show_selected_comp(BITMAP* buffer, int selected_competence) {
    //printf("selected_competence = %d\n", selected_competence);
    const int pad = 10;
    int x = pad;
    int y = SCREEN_H-pad-(int)(442*0.7);
    /*
    if (x > 985*0.7 || x < 280*0.7 || y > -500 || y < -600) return 0;
    if (x < 400*0.7 && x > 280*0.7) num_competence = 1;
    if (x < 530*0.7 && x > 410*0.7) num_competence = 2;
    if (x < 660*0.7 && x > 545*0.7) num_competence = 3;
    if (x < 800*0.7 && x > 675*0.7) num_competence = 4;
    if (x < 985*0.7 && x > 810*0.7) num_competence = 5;
    */
    switch (selected_competence) {
        case 1:
            for (int i=0; i<5; i++) {
                rect(buffer, x+280*0.7+i, y+45+i, x+400*0.7-i, y+100+45-i, makecol(255,255,0));
            } break;
        case 2:
            for (int i=0; i<5; i++) {
                rect(buffer, x+410*0.7+i, y+45+i, x+530*0.7-i, y+100+45-i, makecol(255,255,0));
            }break;
        case 3:
            for (int i=0; i<5; i++) {
                rect(buffer, x+545*0.7+i, y+45+i, x+660*0.7-i, y+100+45-i, makecol(255,255,0));
            }break;
        case 4:
            for (int i=0; i<5; i++) {
                rect(buffer, x+675*0.7+i, y+45+i, x+800*0.7-i, y+100+45-i, makecol(255,255,0));
            }break;
        case 5:
            for (int i=0; i<5; i++) {
                rect(buffer, x+810*0.7+i, y+45+i, x+985*0.7-i, y+100+45-i, makecol(255,255,0));
            }break;
        default: return;
    }
}

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void attack_statut(Perso* self, int num_competence) {
    //Competences de protection
    if (strcmp(self->classe.competences[num_competence].nom_competence, "Mur de lianes")==0 ||
        strcmp(self->classe.competences[num_competence].nom_competence, "Bulle d'eau")==0)
        {
        self->protection = true;
    }
    //Competences de soin
    if (strcmp(self->classe.competences[num_competence].nom_competence, "Aromatherapie")==0 ||
        strcmp(self->classe.competences[num_competence].nom_competence, "Recolte")==0 ||
        strcmp(self->classe.competences[num_competence].nom_competence, "Eau de vie")==0)
        {
        int soin=0;
        soin = self->classe.competences[num_competence].degat*self->classe.foi;
        self->pv_actuels += soin;
        if (self->pv_actuels > self->pv_actuels) self->pv_actuels = self->pv_actuels;
    }
    //Compétence de boost
    if (strcmp(self->classe.competences[num_competence].nom_competence, "Rage")==0) {
        self->boost_modifier = 1.5;
    }
}



void attack(Perso* attaquant, Perso* defenseur, int num_competence) {
    // 1) Vérification du coût en mana/PA
    t_competence *spell = &attaquant->classe.competences[num_competence];
    if (attaquant->p_attaque < spell->p_attaque)
        return;

    // 2) Calcul des coordonnées écran isométriques du défenseur
    const int origin_x = SCREEN_W / 2;
    const int offset_y = SCREEN_H / 2 - TILE_HEIGHT * PLAT_Y / 2;
    int iso_x = (defenseur->x - defenseur->y) * (TILE_WIDTH / 2) + origin_x;
    int iso_y = (defenseur->x + defenseur->y) * (TILE_HEIGHT / 2) + offset_y;

    // 3) Affichage de l’animation de la compétence (3 frames)
    for (int f = 0; f < 3; f++) {
        // On dessine directement sur l’écran.
        // Si vous avez un back‐buffer nommé 'buffer', remplacez 'screen' par 'buffer'
        draw_sprite(screen,
                    spell->sprite[f],
                    iso_x,
                    iso_y - spell->sprite[f]->h / 2  // décalage pour que l’anim "flotte" au‐dessus
        );
        rest(100);  // 100 ms entre chaque frame
    }

    // 4) Si c’est un sort de statut (type_stat == 'N'), on applique juste l’effet
    if (spell->type_stat == 'N') {
        attack_statut(attaquant, num_competence);
        return;
    }

    // 5) Sinon on vérifie la portée et on calcule les dégâts
    int dx = abs(attaquant->x - defenseur->x);
    int dy = abs(attaquant->y - defenseur->y);
    if (dx + dy > spell->portee)
        return;

    // Dégâts de base + stat
    float total = (float)spell->degat;
    switch (spell->type_stat) {
        case 'F': total += attaquant->classe.foi;         break;
        case 'S': total += attaquant->classe.force;       break;
        case 'I': total += attaquant->classe.intelligence;break;
        case 'D': total += attaquant->classe.dexterite;   break;
    }
    // Application de la résistance de l’adversaire
    switch (spell->type_degat) {
        case 'C': total *= defenseur->classe.r_contandant; break;
        case 'T': total *= defenseur->classe.r_tranchant;  break;
        case 'P': total *= defenseur->classe.r_percant;    break;
        case 'E': total *= defenseur->classe.r_eau;        break;
        case 'F': total *= defenseur->classe.r_feu;        break;
        case 'S': total *= defenseur->classe.r_terre;      break;
    }
    total *= attaquant->boost_modifier;
    int dmg = (int)total;

    // 6) On consomme les PA et on inflige les PV
    attaquant->p_attaque -= spell->p_attaque;
    defenseur->pv_actuels   -= dmg;
}

int found_player(Game game, int x, int y) {
    for (int i=0; i<NB_JOUEURS; i++) {
        if (x == game.players[i].x && y == game.players[i].y) {
            return i;
        }
    }
    return -1;
}

bool est_case_valide_BFS(int x, int y, int map[PLAT_Y][PLAT_X]) {
    return x >= 0 && x < PLAT_X && y >= 0 && y < PLAT_Y && map[y][x] == 0;
}

bool verif_bfs(Game game, int origin_x, int origin_y, int dest_x, int dest_y, int pm_joueur, Node map_path[PLAT_Y][PLAT_X], int* len_path) {
//bool verif_bfs(Game game, int origin_x, int origin_y, int dest_x, int dest_y, int pm_joueur) {

    if (!est_case_valide_BFS(dest_x, dest_y, game.plateau)) return false;

    int visited[PLAT_Y][PLAT_X] = {0};
    Node prev[PLAT_Y][PLAT_X];

    // Marquer obstacles
    for (int y = 0; y < PLAT_Y; y++) {
        for (int x = 0; x < PLAT_X; x++) {
            if (game.plateau[y][x] != 0)
                visited[y][x] = -1;
        }
    }

    // Marquer joueurs
    for (int i = 0; i < 4; i++) {
        if (game.players[i].x == origin_x && game.players[i].y == origin_y)
            continue;
        visited[game.players[i].y][game.players[i].x] = -1;
    }

    int queue_x[MAX_NODE], queue_y[MAX_NODE];
    int front = 0, back = 0;
    queue_x[back] = origin_x;
    queue_y[back] = origin_y;
    back++;
    visited[origin_y][origin_x] = 1;

    bool found = false;
    int directions[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};

    while (front < back && !found) {
        int ux = queue_x[front];
        int uy = queue_y[front];
        front++;

        for (int i = 0; i < 4; i++) {
            int vx = ux + directions[i][0];
            int vy = uy + directions[i][1];

            if (est_case_valide_BFS(vx, vy, game.plateau) && visited[vy][vx] == 0) {
                visited[vy][vx] = 1;
                prev[vy][vx].x = ux;
                prev[vy][vx].y = uy;
                queue_x[back] = vx;
                queue_y[back] = vy;
                back++;

                if (vx == dest_x && vy == dest_y) {
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found) return false;

    // Remonter le chemin
    int len = 0;
    int cx = dest_x, cy = dest_y;
    //printf("%d , %d\n", cx, cy);
    Node path_temp[PLAT_Y][PLAT_X]; // [y][x]
    for (int i = 0; i < PLAT_Y; i++) {
        for (int j = 0; j < PLAT_X; j++) {
            path_temp[i][j].x = prev[i][j].x;
            path_temp[i][j].y = prev[i][j].y;
        }
    }
    while (!(cx == origin_x && cy == origin_y)) {
        Node p = prev[cy][cx];
        //printf("%d , %d\n", p.x, p.y);
        cx = p.x;
        cy = p.y;
        len++;
    }
    //printf("\n\n");

    for (int i = 0; i < PLAT_Y; i++) {
        for (int j = 0; j < PLAT_X; j++) {
            map_path[i][j].x = path_temp[i][j].x;
            map_path[i][j].y = path_temp[i][j].y;
        }
    }

    if (len <= pm_joueur) {
        *len_path = len;
        return true;
    }
    return false;
}

void inversion_chemin(Node map_path[PLAT_Y][PLAT_X], int len, Node path[len+1], int dest_x, int dest_y, int origin_x, int origin_y) {

    int i=0;
    int cx = dest_x, cy = dest_y;
    path[0].x = origin_x;
    path[0].y = origin_y;
    path[len].x = cx;
    path[len].y = cy;
    while (!(cx == origin_x && cy == origin_y)) {
        Node p2 = map_path[cy][cx];
        cx = p2.x;
        cy = p2.y;
        i++;
        path[len-i]=p2;
    }
    for (int j = 0; j < len+1; j++) {
        printf("%d , %d\n", path[j].x, path[j].y);
    }
}

bool can_move(Game game, const Perso self, const int x_dest, const int y_dest,Node map_path[PLAT_Y][PLAT_X],int len_path) {
    //printf("init_verif\n");
    //La case cliquée se trouve-t-elle dans le plateau ?
    if (x_dest < 0 || y_dest < 0) return false;
    //printf("dans le plateau\n");
    //La case cliquée est-elle sur un obstacle ?
    if (game.plateau[x_dest][y_dest] !=0) return false;
    //printf("pas d'obstacle\n");
    //La case cliquée est-elle sur un joueur ?
    if (found_player(game, x_dest, y_dest)!=-1) return false;
    //printf("pas de joueur\n");
    //Verification du déplacement avec un BFS
    //printf("case dest valide\n");
    if (!verif_bfs(game, self.x, self.y, x_dest, y_dest, self.pm_restant,map_path,&len_path)) return false;
    //printf("BFS true\n");
    //Toutes les vérifications sont validées
    return true;
}
void iso_to_screen(int x, int y, int *screen_x, int *screen_y) {
    int origin_x = SCREEN_W/2;
    int offset_y = SCREEN_H / 2 - TILE_HEIGHT  * PLAT_Y / 2;
    *screen_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
    *screen_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;
    //if (iso_x + TILE_WIDTH > 0 && iso_x < SCREEN_W && iso_y + TILE_HEIGHT > 0 && iso_y < SCREEN_H);
}
void deplacement(Game* game, Perso* self,
                 const int x_dest,
                 const int y_dest,
                 Node map_path[PLAT_Y][PLAT_X],
                 int len_path, int num_joueur)
{
    //Init position initiale
    int origin_x = self->x;
    int origin_y = self->y;
    // Création d’un back-buffer local
    BITMAP* buffer = create_bitmap(screen->w, screen->h);
    if (!buffer) buffer = screen;  // fallback si échec

    // Reconstruction du chemin
    Node path[len_path+1];
    inversion_chemin(map_path, len_path, path,
                     x_dest, y_dest,
                     self->x, self->y);

    // Parcours des segments
    for (int i = 0; i < len_path; i++) {
        int dx = path[i+1].x - path[i].x;
        int dy = path[i+1].y - path[i].y;

        // Choix des deux frames selon la direction
        int f0, f1;
        if (dx > 0) {
            // →  (frames 3 & 4)
            f0 = 2;  f1 = 3;
        }
        else if (dx < 0) {
            // ←  (frames 5 & 6)
            f0 = 4;  f1 = 5;
        }
        else if (dy > 0) {
            // ↓  (frames 1 & 2)
            f0 = 0;  f1 = 1;
        }
        else {
            // ↑  (frames 7 & 8)
            f0 = 6;  f1 = 7;
        }

        // Animation des deux frames
        for (int frame = 0; frame < 2; frame++) {
            clear_bitmap(buffer);
            int x_screen, y_screen;
            iso_to_screen(path[i].x, path[i].y, &x_screen, &y_screen);
            draw_sprite(buffer,
                        self->classe.sprite[(frame == 0) ? f0 : f1],
                        x_screen, y_screen);
            if (buffer != screen) {
                blit(buffer, screen, 0,0, 0,0, screen->w, screen->h);
            }
            rest(100);
        }

        // Mise à jour logique de la position
        self->x = path[i+1].x;
        self->y = path[i+1].y;
    }

    // S’assurer d’être exactement à la destination
    self->x = x_dest;
    self->y = y_dest;
    self->pm_restant -= len_path;
    game->plateau[y_dest][x_dest] = TILE_COUNT + num_joueur;
    game->plateau[origin_y][origin_x] = TILE_COUNT - num_joueur;
    printf(" position : (%d, %d)\n", self->x, self->y);

    // Libération du buffer local
    if (buffer != screen) destroy_bitmap(buffer);
}

void action(Game* game, Perso* self, const int num_competence, const int action_x, const int action_y, int num_joueur) {
    if (num_competence<=0 || num_competence>5) return;
    printf("log action: %d, (%d,%d)", num_competence, action_x, action_y);
    if (num_competence == 5) {
        //Appel Can_move
        int len_path;
        Node map_path[PLAT_Y][PLAT_X];
        for (int i = 0; i < PLAT_Y; i++) {
            for (int j = 0; j < PLAT_X; j++) {
                map_path[i][j].x = -1;
                map_path[i][j].y = -1;
            }
        }
    }

    if (num_competence == 5) {
        //Appel Can_move
        int len_path;
        Node map_path[PLAT_Y][PLAT_X];
        for (int i = 0; i < PLAT_Y; i++) {
            for (int j = 0; j < PLAT_X; j++) {
                map_path[i][j].x = -1;
                map_path[i][j].y = -1;
            }
        }
        printf("Debut can_move\n");
        if (can_move(*game, *self, action_x, action_y,map_path,len_path)) {
            //Appel fct déplacement
            printf("debut deplacement\n");
            deplacement(game, self, action_x, action_y, map_path, len_path, num_joueur);
                //Appel fct qui transfère les données au réseau (compétence 5, x_dest, x_dest)
        }
    }
    else {
        attack(self,&game->players[found_player(*game,action_x,action_y)],num_competence);
    }
}

void translation_to_iso(int*x,int* y) {
    int origin_x = SCREEN_W / 2;  // Utilise SCREEN_W et SCREEN_H
    int offset_y = SCREEN_H / 2 - TILE_HEIGHT  * PLAT_Y / 2;
    float x_fix =mouse_x-origin_x;
    float y_fix =mouse_y-offset_y;
    float hw=TILE_WIDTH/2.0f;
    float hh=TILE_HEIGHT/2.0f;
    float fx=(x_fix/hw+y_fix/hh) / 2.0f;
    float fy =(y_fix/hh-x_fix/hw) / 2.0f;
    int x_temp =(int)(fx-0.5f);
    int y_temp =(int)(fy-0.5f);
    if (y_temp>5) y_temp+=1;
    if(x_temp>=0 && x_temp<PLAT_X && y_temp>=0 && y_temp<PLAT_Y) {
        *x = x_temp;
        *y = y_temp;
        printf("x : %d\ny : %d\n", *x,*y);
    }else {
        *y=-1;
        *x=-1;
    }
}

int change_music(const char *filename)
{
    static SAMPLE *current = NULL;
    SAMPLE *next;

    next = load_sample(filename);
    if (!next) {
        allegro_message("Erreur : impossible de charger '%s'\n", filename);
        return -1;
    }

    if (musique) {
        stop_sample(musique);
        destroy_sample(musique);
    }

    musique = next;
    if (play_sample(musique, 255, 128, 1000, 1) < 0) {
        allegro_message("Erreur : impossible de jouer '%s'\n", filename);
        return -1;
    }

    return 0;
}

void save_map_to_txt (int plateau[PLAT_X][PLAT_Y]) {
    FILE* p_map = fopen("../Projet/Graphismes/Map/Fichiers textes/MapAleatoire.txt", "w");
    for (int x = 0; x < PLAT_X; x++) {
        for (int y = 0; y < PLAT_Y; y++) {

            fprintf(p_map, "%d ", plateau[x][y]);
        }
        fprintf(p_map, "\n");
    }
    fclose(p_map);
}
//void import_terrainJeu_Via_Fichier_texte(int theme, Map* map, int carte_carthesien[PLAT_X][PLAT_Y]) {
void import_terrainJeu_Via_Fichier_texte(Game* game){
    char filename[100];

    //Chargement des tuiles du theme choisi
    for (int i=0; i<3; i++){
        snprintf(filename, sizeof(filename),"../Projet/Graphismes/Map/Tuiles/%d/%d.bmp",(game->theme)+1, i+1);
        printf("%s\n",filename);
        game->map.images[i] = charger_et_traiter_image(filename, 64, 64);
        printf("Tuile %d chargé\n",i+1);
    }
    //Chargement de l'arriere plan du theme choisi
    snprintf(filename, sizeof(filename),"../Projet/Graphismes/Map/Background/%d.bmp",game->theme+1);
    game->map.background = charger_et_traiter_image(filename, SCREEN_W, SCREEN_H);

    switch (game->theme) {
        case 0:
            //Changement de musique
            change_music("../Projet/Musiques/Electric-Breeze-1.wav");
            break;
        case 1:
            //Changement de musique
            change_music("../Projet/Musiques/Volcanic-Arcade1.wav");
            break;
        case 2:
            //Changement de musique
            change_music("../Projet/Musiques/MapSand.wav");
            break;
        case 3:
            //Changement de musique
            change_music("../Projet/Musiques/Cloud-Tornado.wav");
            break;
    }

    //Vérification tuiles chargée correctement
    for (int i = 0; i < TILE_COUNT; i++) {
        if (!game->map.images[i]) {
            allegro_message("Erreur : Tuile %d introuvable !", i);
            exit(EXIT_FAILURE);
        }
    }

    //Vérification de l'arriere plan chargée correctement
    if (!game->map.background) {
        allegro_message("Erreur : Impossible de charger le fond d'écran !");
        exit(EXIT_FAILURE);
    }
    printf("Background Width: %d, Height: %d\n", game->map.background->w, game->map.background->h);
    /*
    FILE* p_fichier_map = NULL;
    //Chargement de la map du fichier txt
    p_fichier_map = fopen("../Projet/Graphismes/Map/Fichiers textes/MapAleatoire.txt", "r");
    //transfert entre fichier texte et la structure map
    for (int x = 0; x < PLAT_Y; x++) {
        for (int y = 0; y < PLAT_X; y++) {
            fscanf(p_fichier_map, "%d ", game->plateau[x][y]);
        }
        fscanf(p_fichier_map, "\n");
    }

    fclose(p_fichier_map);*/
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
        Sleep(200); // version windows
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

void dessiner_losange(BITMAP* buffer, int cx, int cy, int w, int h, int fill_color, int border_color) {
    int points[8] = {
        cx,         cy - h / 2,
        cx + w / 2, cy,
        cx,         cy + h / 2,
        cx - w / 2, cy
    };
    set_trans_blender(0, 0, 0, 100);
    drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    polygon(buffer, 4, points, fill_color);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    for (int i = 0; i < 4; i++) {
        line(buffer, points[i*2], points[i*2+1], points[((i+1)%4)*2], points[((i+1)%4)*2+1], border_color);
    }
}

void afficher_portee(BITMAP * buffer,Game game, Perso joueur, int x, int y, int iso_x, int iso_y) {
    if (game.portee[x][y]==1) {
        dessiner_losange(buffer, iso_x+32, iso_y+16+20, TILE_WIDTH, TILE_HEIGHT, makecol(255,255,0), makecol(255,255,255));
    }
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

void dishtra(Game * game, int x, int y, int portee, int pas) {
    pas += 1;
    game->portee[y][x]=1;
    if (pas > portee) return;

    // haut
    if (y-1 >= 0) {
        if (game->plateau[y-1][x] == 0 && game->portee[y-1][x] == 0) {
            dishtra(game, x, y-1, portee, pas);
        }
    }
    // droite
    if (x+1 < PLAT_X) {
        if (game->plateau[y][x+1] == 0 && game->portee[y][x+1] == 0) {
            dishtra(game, x+1, y, portee, pas);
        }
    }
    // bas
    if (y+1 < PLAT_Y) {
        if (game->plateau[y+1][x] == 0 && game->portee[y+1][x] == 0) {
            dishtra(game, x, y+1, portee, pas );
        }
    }
    // gauche
    if (x-1 >= 0) {
        if (game->plateau[y][x-1] == 0 && game->portee[y][x-1] == 0) {
            dishtra(game, x-1, y, portee, pas);
        }
    }
}

void update_portee(Game * game, Perso player, int num_competence) {
    init_portee(game);
    int portee;
    if (num_competence == 5 )
        portee = player.pm_restant;
    else
        portee = player.classe.competences[num_competence-1].portee;

    printf("portee : %d\n", portee);

    int x = player.x;
    int y = player.y;
    int pas =0;
    dishtra(game, x, y, portee, pas);
}

void detection_competence (Game * game, Perso player,int * num_competence) {
    const int pad = 10;
    int x = mouse_x-pad;
    int y = mouse_y-pad-SCREEN_H-(int)(442*0.7);
    int buff = *num_competence;
    if (x > 985*0.7 || x < 280*0.7 || y > -500 || y < -600) return;
    if (x < 400*0.7 && x > 280*0.7) *num_competence = 1;
    if (x < 530*0.7 && x > 410*0.7) *num_competence = 2;
    if (x < 660*0.7 && x > 545*0.7) *num_competence = 3;
    if (x < 800*0.7 && x > 675*0.7) *num_competence = 4;
    if (x < 985*0.7 && x > 810*0.7) *num_competence = 5;

    if (*num_competence != buff) { // nouvelle competence
        update_portee(game, player, *num_competence);
    }
}
void barre_jeu(BITMAP* buffer, BITMAP* icon, t_classe classe, int selected_competence)
{
    if (!icon) return;
    const int pad = 10;
    int x = pad;
    int y = SCREEN_H - icon->h - pad;
    draw_sprite(buffer, icon, x, y);

    draw_sprite(buffer, classe.competences[0].sprite[2] , x+205, y+80);
    draw_sprite(buffer, classe.competences[1].sprite[2] , x+298, y+80);
    draw_sprite(buffer, classe.competences[2].sprite[2] , x+392, y+80);
    draw_sprite(buffer, classe.competences[3].sprite[2] , x+485, y+80);

    draw_sprite(buffer, classe.sprite[8] , x-4, y+11);
}

void bouton_next(BITMAP* buffer, BITMAP* icon) {
    if (!icon) return;
    const int pad = 10;
    int x = SCREEN_W - icon->w - pad;
    int y = SCREEN_H - icon->h - pad;
    draw_sprite(buffer, icon, x, y);
}

void show_graphique(Game game, int n_turns, int i, BITMAP* buffer, BITMAP* curseur,BITMAP* panneau_bas_gauche,BITMAP* next_button,  int selected_competence)
{
    // --- Fond ---
    if (game.map.background) {
        stretch_blit(game.map.background, buffer,
                     0, 0,
                     game.map.background->w, game.map.background->h,
                     0, 0,
                     SCREEN_W, SCREEN_H);
    }

    // --- Tuiles isométriques ---

    int origin_x = SCREEN_W/2;
    int offset_y = SCREEN_H / 2 - TILE_HEIGHT  * PLAT_Y / 2;

    for (int y = 0; y < PLAT_Y; y++) {
        for (int x = 0; x < PLAT_X; x++) {
            int id = game.plateau[y][x];
            if ( id < TILE_COUNT && game.map.images[id]) { // case vide
                int iso_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
                int iso_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;
                if (iso_x + TILE_WIDTH > 0 && iso_x < SCREEN_W && iso_y + TILE_HEIGHT > 0 && iso_y < SCREEN_H) {
                    draw_sprite(buffer, game.map.images[id],iso_x, iso_y);
                    afficher_portee(buffer,game, game.players[i], x, y, iso_x,iso_y);
                }
            } else {
                int iso_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
                int iso_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;
                if (iso_x + TILE_WIDTH > 0 && iso_x < SCREEN_W && iso_y + TILE_HEIGHT > 0 && iso_y < SCREEN_H) {
                    draw_sprite(buffer, game.map.images[0],iso_x, iso_y);
                    afficher_portee(buffer,game, game.players[i], x, y, iso_x,iso_y);
                    draw_sprite(buffer, game.players[id-TILE_COUNT].classe.sprite[0],iso_x, iso_y-game.players[id-TILE_COUNT].classe.sprite[0]->h/3);
                }
            }
        }
    }

    // --- Affichage bas-gauche via notre helper ---
    barre_jeu(buffer, panneau_bas_gauche, game.players[i].classe, selected_competence);
    show_selected_comp(buffer, selected_competence);
    bouton_next(buffer,next_button);


    // --- Curseur ---
    stretch_sprite(buffer, curseur,
                   mouse_x, mouse_y,
                   32, 32);

    // --- Envoi à l'écran ---
    blit(buffer, screen,
         0, 0,   // src x,y
         0, 0,   // dst x,y
         SCREEN_W, SCREEN_H);
}

void next_cliqued(int * next) {
    const int pad = 10;
    int x = SCREEN_W - 651*0.5 - pad;
    int y = SCREEN_H - 342*0.5 - pad;

    if (mouse_x > x && mouse_x < x+651*0.5 && mouse_y > y && mouse_y < y+342*0.5) {
        *next = 1;
        printf("next\n");
        Sleep(200);
    }
}

void tour_graphique(Game * game, int i, int * competence,  int * next, int * quit ) {
    // clic sur la grille
    int x,y;
    if (mouse_b & 1) {
        next_cliqued(next);

        translation_to_iso(&x, &y);
        if (x != -1 && y != -1) {
            action(game, &game->players[i], *competence, x, y, i);
        } else {

        detection_competence(game,game->players[i], competence);
        }
    }


    // clavier
    if (keypressed()) {
        int keycode = readkey();
        int k = keycode >> 8;
        char ch = keycode & 0xFF;
        if (k == KEY_ESC) {
            *next = 1;
            *quit = 1;
            change_music("../Projet/Musiques/Menus.wav");
            return;
        }
    }
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
    /*
    game->map.background = load_bitmap("../DATA/GAME/MAP/BACKGROUND/2.bmp", NULL);
    if (!game->map.background) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < TILE_COUNT; i++) {
        char path[100];
        sprintf(path, "../DATA/GAME/MAP/TUILES/2/%d.bmp", i+1);
        game->map.images[i] = charger_et_traiter_image(path, 64, 64);
    }*/
    import_terrainJeu_Via_Fichier_texte(game);




    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Charger l'image du bouton next

    BITMAP* next_button = charger_et_traiter_image(
            "../Projet/Graphismes/Menus/Boutons/NEXT.bmp",
            651*0.5,342*0.5
        );


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

    BITMAP* panneau_bas_gauche = charger_et_traiter_image(
            "../Projet/Graphismes/Interface/BarreDeJeu/1.bmp",
            1024*0.7,459*0.7
        );


    // ===
    long int received;
    char LAN_buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    int n_turns = 0;
    int next = 0;
    int selected_competence=-1;
    show(*game, 0, num); // log
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            selected_competence=-1;
            init_portee(game);
            if (num == i) {
                while (!next) {
                    show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche, next_button, selected_competence); // affiche l'ecrant de jeu
                    //tour_graphique(game, i, &next ); // verifie les actions du joueur et joue joue
                    rest(10);
                }
                tour(game, i, LAN_buffer); // le joueur joue
                send(sock, LAN_buffer, strlen(LAN_buffer), 0); // les données sont envoyées
                printf("[Game] Data sent\n");
            } else {
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence); // affiche l'ecrant de jeu
                get_data(sock, &received, LAN_buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, LAN_buffer); // on traite les données des autres joueurs
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence); // affiche l'ecrant de jeu
            }
        }
    }
    destroy_bitmap(panneau_bas_gauche);
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
    /*
    game->map.background = load_bitmap("../DATA/GAME/MAP/BACKGROUND/2.bmp", NULL);
    if (!game->map.background) {
        allegro_message("Erreur lors du chargement de l'arrière-plan !");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < TILE_COUNT; i++) {
        char path[100];
        sprintf(path, "../DATA/GAME/MAP/TUILES/2/%d.bmp", i+1);
        game->map.images[i] = charger_et_traiter_image(path, 64, 64);
    }*/
    import_terrainJeu_Via_Fichier_texte(game);

    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }

    // Appliquer la transparence sur le curseur
    appliquer_transparence_curseur(curseur);

    // Dimensions désirées du curseur (ex : 32x32)

    BITMAP* panneau_bas_gauche = charger_et_traiter_image(
            "../Projet/Graphismes/Interface/BarreDeJeu/1.bmp",
            1024*0.7,459*0.7
        );
    BITMAP* next_button = charger_et_traiter_image(
            "../Projet/Graphismes/Menus/Boutons/NEXT.bmp",
            651*0.5,342*0.5
        );



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
    int selected_competence=-1;
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            selected_competence=-1;
            next = 0;
            init_portee(game);
            while (!next) {
                show_graphique(*game,n_turns,i, buffer, curseur, panneau_bas_gauche,next_button, selected_competence); // affiche l'ecrant de jeu
                tour_graphique(game, i,&selected_competence, &next, &quit); // verifie les actions du joueur et joue joue
                rest(10);
            }
            //check_victory(game, &quit);
            if (quit) break;
        }
    }
    destroy_bitmap(panneau_bas_gauche);
    destroy_bitmap(buffer);
    destroy_bitmap(game->map.background);
    clear_keybuf();
}


// Initialisation et affichage du menu de sélection du nombre de joueurs
void init_nb_players_graphique() {
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

    char * message = "[ INIT ]";
    BITMAP* texte = create_bitmap(8 * 8, 16);
    clear_to_color(texte, makecol(255, 0, 255));
    textprintf_ex(texte, font, 0, 0,
                  makecol(0, 0, 0), -1,
                  "%s", message);
    stretch_sprite(screen, texte,
                   SCREEN_W/2- texte->w*5 /2, 3*SCREEN_H/4,
                    texte->w*5, texte->h*5);
    destroy_bitmap(texte);

    //grille persos
    char chemin_perso[256];
    char chemin_avatar[256];
    for (int i = 0; i < 12; i++) {
        sprintf(chemin_perso, "../Projet/Graphismes/Menus/Select/%d.bmp", i + 1);
        liste_avatar[i] = charger_et_traiter_image(chemin_perso, SCREEN_H/8, SCREEN_H/8);
        liste_big_avatar[i] = charger_et_traiter_image(chemin_perso, SCREEN_H/8*3, SCREEN_H/8*3);
        sprintf(chemin_perso, "../Projet/Graphismes/Menus/Story/%d.bmp", i + 1);
        liste_story[i] = load_bitmap(chemin_perso, NULL);
    }
    // revoir parametres grille 3*4 dans menu_selection_personnages, dans menu.c pour taille

}
