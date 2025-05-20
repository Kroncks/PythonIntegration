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
static BITMAP* sprite_mort = NULL;
int ANIMATION =0;

int REPLAY =0;

void init_tour(Game *game) {
    // Recharge pour chaque joueur :
    //  - points d'attaque = mana * 10
    //  - points de mouvement = endurance
    for (int i = 0; i < NB_JOUEURS; i++) {
        game->players[i].p_attaque  = game->players[i].classe.mana * 10;
        game->players[i].pm_restant = game->players[i].classe.endurance;
    }
    sprintf(game->last_action, "%d %d %d",0, 0, 0);
    game->last_action[strlen(game->last_action)] = '\0';
    printf("INIT last_action : %s\n", game->last_action);
}

void init_coord(Game * game) {
    game->players[0].x = game->players[0].y = 0;
    game->players[1].x = PLAT_X-1; game->players[1].y = PLAT_Y-1;

    game->players[2].x = PLAT_X-1; game->players[2].y = 0;
    game->players[3].x = 0; game->players[3].y = PLAT_Y-1;

    game->plateau[0][0]= TILE_COUNT;
    game->plateau[PLAT_Y-1][PLAT_X-1]= TILE_COUNT+1;

    if (NB_JOUEURS == 4) {
        game->plateau[PLAT_X-1][0]= TILE_COUNT+2;
        game->plateau[0][PLAT_Y-1]= TILE_COUNT+3;
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

    game->theme = rand() % NB_THEMES;

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

int found_player(Game game, int x, int y) {
    for (int i=0; i<NB_JOUEURS; i++) {
        if (x == game.players[i].x && y == game.players[i].y) {
            return i;
        }
    }
    return -1;
}


void attack_statut(Perso* self, int idx) {
    // Sécurité : idx dans [0..3]
    if (idx < 0 || idx >= 4) return;

    const char* nom = self->classe.competences[idx].nom_competence;

    // Compétences de protection
    if (strcmp(nom, "Mur de lianes") == 0 ||
        strcmp(nom, "Bulle d'eau") == 0) {
        self->protection = true;
        //return;
        }

    // Compétences de soin
    if (strcmp(nom, "Aromatherapie") == 0 ||
        strcmp(nom, "Recolte")       == 0 ||
        strcmp(nom, "Eau de vie")    == 0) {
        int soin = self->classe.competences[idx].degat * self->classe.foi;
        self->pv_actuels += soin;
        if (self->pv_actuels > self->classe.pv)
            self->pv_actuels = self->classe.pv*10;
        //return;
        }

    // Compétence de boost
    if (strcmp(nom, "Rage") == 0) {
        self->boost_modifier = 1.5f;
    }
    self->p_attaque -= self->classe.competences[idx].p_attaque;

}


void attack(Game * game, Perso* attaquant, Perso* defenseur, int idx) {
    // 0) Sécurité d'indice
    if (idx < 0 || idx >= 4) return;

    t_competence *spell = &attaquant->classe.competences[idx];

    // 0.1) Empêcher les sorts de statut sur une autre cible
    if (spell->type_stat == 'N' && defenseur != attaquant)
        return;

    // 0.2) Empêcher les attaques de dégâts sur soi (déjà en place)
    if (spell->type_stat != 'N' && defenseur == attaquant)
        return;

    // 1) Vérif coût PA
    if (attaquant->p_attaque < spell->p_attaque)
        return;

    // 2) Coordonnées écran isométriques de l’attaquant
    const int origin_x = SCREEN_W/2;
    const int offset_y = SCREEN_H/2 - TILE_HEIGHT*PLAT_Y/2;

    // Sort de statut
    if (spell->type_stat == 'N') {
        int atk_x = (attaquant->x - attaquant->y)*(TILE_WIDTH/2) + origin_x;
        int atk_y = (attaquant->x + attaquant->y)*(TILE_HEIGHT/2) + offset_y;
        for (int f = 0; f < 3; f++) {
            draw_sprite(screen,
                        spell->sprite[f],
                        atk_x,
                        atk_y - spell->sprite[f]->h/2);
            rest(100);
        }
        attack_statut(attaquant, idx);
        return;
    }

    // 5) Sinon on vérifie la portée et on calcule les dégâts
    int dx = abs(attaquant->x - defenseur->x);
    int dy = abs(attaquant->y - defenseur->y);
    if (dx + dy > spell->portee) return;

    // Attaque classique
    int iso_x = (defenseur->x - defenseur->y)*(TILE_WIDTH/2) + origin_x;
    int iso_y = (defenseur->x + defenseur->y)*(TILE_HEIGHT/2) + offset_y;
    for (int f = 0; f < 3; f++) {
        draw_sprite(screen,
                    spell->sprite[f],
                    iso_x,
                    iso_y - spell->sprite[f]->h/2);
        rest(100);
    }
    // Dégâts de base + stat
    float total = (float)spell->degat;
    switch (spell->type_stat) {
        case 'F': total += attaquant->classe.foi;         break;
        case 'S': total += attaquant->classe.force;       break;
        case 'I': total += attaquant->classe.intelligence;break;
        case 'D': total += attaquant->classe.dexterite;   break;
    }
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
    if (defenseur->protection) {
        defenseur->protection = false;
        dmg = 0;
    }
    // 5) Consommation PA et application dégâts
    attaquant->p_attaque  -= spell->p_attaque;
    defenseur->pv_actuels -= dmg;
    if (defenseur->pv_actuels <= 0) {
        defenseur->pv_actuels = 0;
        game->poduim[game->nb_morts] = *defenseur;
        game->nb_morts+=1;
    }
}


// ====================================================================================



void dishtra(Game *game, int start_x, int start_y, int portee, int pas, int num_competences) {
    typedef struct { int x, y, dist; } Node;
    Node queue[PLAT_X * PLAT_Y];
    int front = 0, rear = 0;

    game->portee[start_x][start_y] = 1;
    game->prev[start_x][start_y] = (Coord){-1, -1};
    queue[rear++] = (Node){start_x, start_y, 0};

    while (front < rear) {
        Node curr = queue[front++];
        if (curr.dist >= portee) continue;

        int dx[4] = {1, -1, 0, 0};
        int dy[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= PLAT_X || ny >= PLAT_Y) continue;
            if (game->plateau[nx][ny] != 0 || (num_competences != 5 && game->plateau[nx][ny] > TILE_COUNT)) continue;
            if (game->portee[nx][ny]) continue;

            game->portee[nx][ny] = 1;
            game->prev[nx][ny] = (Coord){curr.x, curr.y};
            queue[rear++] = (Node){nx, ny, curr.dist + 1};
        }
    }
}


int get_path(Game *game, int x, int y, Coord path[], int max_len) {
    int len = 0;
    while (x != -1 && y != -1 && len < max_len) {
        path[len++] = (Coord){x, y};
        Coord p = game->prev[x][y];
        x = p.x;
        y = p.y;
    }
    for (int i = 0; i < len / 2; i++) {
        Coord tmp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = tmp;
    }
    return len;
}


void update_portee(Game *game, Perso player, int num_competence) {
    init_portee(game);
    int portee = (num_competence == 5)? player.pm_restant : player.classe.competences[num_competence - 1].portee;

    int x = player.x;
    int y = player.y;
    dishtra(game, x, y, portee, 0, num_competence);
}

void afficher_portee(BITMAP * buffer, Game game, Perso joueur, int x, int y, int iso_x, int iso_y) {
    if (x >= 0 && x < PLAT_X && y >= 0 && y < PLAT_Y && game.portee[x][y] == 1) {
        dessiner_losange(buffer, iso_x+32, iso_y+16+20, TILE_WIDTH, TILE_HEIGHT, makecol(255,255,0), makecol(255,255,255));
    }
}

void translation_to_iso(int* x, int* y) {
    const int origin_x = SCREEN_W/2;
    const int origin_y = SCREEN_H/2 - TILE_HEIGHT*PLAT_Y/2;
    const int click_offset = 14;

    float mx = mouse_x - origin_x;
    float my = mouse_y - (origin_y + click_offset);
    float half_w = TILE_WIDTH  / 2.0f;
    float half_h = TILE_HEIGHT / 2.0f;

    float fx = (mx/half_w + my/half_h) * 0.5f;
    float fy = (my/half_h - mx/half_w) * 0.5f;

    int tx = (int)floorf(fx);
    int ty = (int)floorf(fy);

    if (tx >= 0 && tx < PLAT_X && ty >= 0 && ty < PLAT_Y) {
        *x = tx;
        *y = ty;
    } else {
        *x = -1;
        *y = -1;
    }
}


// ========================================


void deplacement(Game *game, Perso *self, const int x_dest, const int y_dest, int num_joueur) {
    int origin_x = self->x;
    int origin_y = self->y;

    BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) buffer = screen;

    Coord path[PLAT_X * PLAT_Y];
    int steps = get_path(game, x_dest, y_dest, path, PLAT_X * PLAT_Y);

    const int originScrX = SCREEN_W / 2;
    const int originScrY = SCREEN_H / 2 - TILE_HEIGHT * PLAT_Y / 2;

    for (int s = 0; s < steps - 1; s++) {
        if (game->map.background) {
            stretch_blit(game->map.background, buffer,
                         0, 0, game->map.background->w, game->map.background->h,
                         0, 0, SCREEN_W, SCREEN_H);
        }

        // Affichage du plateau
        for (int y = 0; y < PLAT_Y; y++) {
            for (int x = 0; x < PLAT_X; x++) {
                int id = game->plateau[x][y];
                int iso_x = (x - y) * (TILE_WIDTH / 2) + originScrX;
                int iso_y = (x + y) * (TILE_HEIGHT / 2) + originScrY;

                if (iso_x + TILE_WIDTH <= 0 || iso_x >= SCREEN_W ||
                    iso_y + TILE_HEIGHT <= 0 || iso_y >= SCREEN_H)
                    continue;

                draw_sprite(buffer, game->map.images[id < TILE_COUNT ? id : 0], iso_x, iso_y);

                if (id >= TILE_COUNT) {
                    int pidx = id - TILE_COUNT;
                    if (pidx != num_joueur) {
                        Perso *pl = &game->players[pidx];
                        BITMAP *spr = (pl->pv_actuels > 0) ? pl->classe.sprite[0] : sprite_mort;
                        int oy = (spr == sprite_mort) ? spr->h / 2 : spr->h / 3;
                        draw_sprite(buffer, spr, iso_x, iso_y - oy);
                    }
                }
            }
        }

        // Animation entre les cases
        int dx = path[s + 1].x - path[s].x;
        int dy = path[s + 1].y - path[s].y;
        int f0, f1;
        if (dx > 0)      { f0 = 0; f1 = 1; }
        else if (dx < 0) { f0 = 2; f1 = 3; }
        else if (dy > 0) { f0 = 4; f1 = 5; }
        else             { f0 = 6; f1 = 7; }

        int px = path[s].x, py = path[s].y;
        int iso_x = (px - py) * (TILE_WIDTH / 2) + originScrX;
        int iso_y = (px + py) * (TILE_HEIGHT / 2) + originScrY;

        for (int frame = 0; frame < 2; frame++) {
            BITMAP *spr = self->classe.sprite[(frame == 0) ? f0 : f1];
            draw_sprite(buffer, spr, iso_x, iso_y - spr->h / 3);
            blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            rest(50);
        }

        self->x = path[s + 1].x;
        self->y = path[s + 1].y;
    }

    self->x = x_dest;
    self->y = y_dest;
    self->pm_restant -= (steps - 1);

    game->plateau[origin_x][origin_y] = 0;
    game->plateau[x_dest][y_dest] = TILE_COUNT + num_joueur;

    if (buffer != screen) destroy_bitmap(buffer);
}


// Fonction appelée sur clic pour exécuter une compétence ou un déplacement
void action(Game *game, Perso *self, int num_competence, int action_x, int action_y, int num_joueur) {
    if (num_competence < 1 || num_competence > 5) return;

    printf("Action : compétence %d sur (%d, %d)\n", num_competence, action_x, action_y);

    if (num_competence == 5) {
        if (game->portee[action_y][action_x] == 1) {
            deplacement(game, self, action_y, action_x, num_joueur);
            update_portee(game, *self, num_competence);
            // mise en buffer de la donnee a envoyer (olalalala)
            sprintf(game->last_action, "%d %d %d",num_competence, action_x, action_y);
            game->last_action[strlen(game->last_action)] = '\0';
            printf("saving last_action : %s\n", game->last_action);
        }
    } else {
        int idx = num_competence - 1;
        int cible = found_player(*game, action_y, action_x);
        if (cible >= 0 && cible < NB_JOUEURS) {
            attack(game, self, &game->players[cible], idx);
            printf("PV du joueur %d (cible) : %d\n", cible, game->players[cible].pv_actuels);
            if (game->players[cible].pv_actuels <= 0) {
                game->plateau[game->players[cible].y][game->players[cible].x] = 0;
            }
            // mise en buffer de la donnee a envoyer (olalalala)
            sprintf(game->last_action, "%d %d %d",num_competence, action_x, action_y);
            game->last_action[strlen(game->last_action)] = '\0';
            printf("saving last_action : %s\n", game->last_action);
        }
    }
}


// ====================================================================================

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



void iso_to_screen(int x, int y, int *screen_x, int *screen_y) {
    int origin_x = SCREEN_W/2;
    int offset_y = SCREEN_H / 2 - TILE_HEIGHT  * PLAT_Y / 2;
    *screen_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
    *screen_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;
    //if (iso_x + TILE_WIDTH > 0 && iso_x < SCREEN_W && iso_y + TILE_HEIGHT > 0 && iso_y < SCREEN_H);
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

void process_data(Game * game, int num, char * data, int * next) {
    int num_competence, action_x, action_y;
    sscanf(data, "%d %d %d", &num_competence, &action_x, &action_y);
    printf(" JOUE player %d : %d (%d,%d)\n", num, num_competence, action_x, action_y);
    if (num_competence == -1) {
        *next = 1;
        return;
    }
    if (num_competence == 5) {
        game->plateau[game->players[num].x][game->players[num].y] = 0; // vide la case
        game->plateau[action_x][action_y] = TILE_COUNT + num;
        game->players[num].x = action_x;
        game->players[num].y = action_y;
    } else {
        action(game, &game->players[num], num_competence, action_x, action_y, num);
    }
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

    for (int i=0; i<NB_JOUEURS; i++) {
        init_player_classe(&game->players[i]);
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
        printf("[GAME] theme : %d\n",game->theme);
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
Coord prev[PLAT_Y][PLAT_X]; // pour reconstruire le chemin

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
void barre_jeu(BITMAP* buffer,
               BITMAP* icon,
               t_classe classe,
               Perso* perso,
               int selected_competence)
{
    if (!icon) return;

    // 1) Positionnement de l'encart
    const int pad = 10;
    int x0 = pad;
    int y0 = SCREEN_H - icon->h - pad;
    draw_sprite(buffer, icon, x0, y0);

    // 2) BARRE DE PV (vert sombre qui diminue)
    const int pv_bar_x = x0 + 203;
    const int pv_bar_y = y0 + 177;
    const int pv_bar_w = 482;  // largeur fixe
    const int pv_bar_h = 30;

    // Récupération et clamp des PV
    int maxPV = perso->classe.pv*10;
    int curPV = perso->pv_actuels;
    if (curPV < 0)     curPV = 0;
    if (curPV > maxPV) curPV = maxPV;

    // Calcul du ratio [0..1]
    float pv_ratio = maxPV > 0
                     ? (float)curPV / (float)maxPV
                     : 0.0f;
    // Conversion en largeur en pixels
    int pv_w = (int)(pv_bar_w * pv_ratio + 0.5f);

    // Dessin de la portion verte sombre
    rectfill(buffer,
             pv_bar_x,
             pv_bar_y,
             pv_bar_x + pv_w,
             pv_bar_y + pv_bar_h,
             makecol(0, 100, 0));

    // 3) ICÔNES DE COMPÉTENCES
    // Frames fixes : 205, 298, 392, 485
    draw_sprite(buffer, classe.competences[0].sprite[2], x0 + 205, y0 + 80);
    draw_sprite(buffer, classe.competences[1].sprite[2], x0 + 298, y0 + 80);
    draw_sprite(buffer, classe.competences[2].sprite[2], x0 + 392, y0 + 80);
    draw_sprite(buffer, classe.competences[3].sprite[2], x0 + 485, y0 + 80);


    // 4) BARRES DE MOUVEMENT (PM) ET D’ACTION (PA)
    int bar_left_x  = x0 + 24;
    int bar_right_x = x0 + icon->w - 30;
    int bar_total_w = bar_right_x - bar_left_x;
    int bar_h       = 22;
    int gap         = 5;
    int pm_bar_y    = y0 + icon->h - 85;
    int pa_bar_y    = pm_bar_y + bar_h + gap;

    // Fond gris pour les deux barres
    rectfill(buffer,
             bar_left_x, pm_bar_y,
             bar_right_x, pa_bar_y + bar_h,
             makecol(80, 80, 80));

    // Barre PM (bleue)
    float pm_ratio = (classe.endurance > 0)
                     ? (float)perso->pm_restant / (float)classe.endurance
                     : 0.0f;
    if (pm_ratio < 0.0f) pm_ratio = 0.0f;
    if (pm_ratio > 1.0f) pm_ratio = 1.0f;
    int pm_w = (int)(bar_total_w * pm_ratio + 0.5f);
    rectfill(buffer,
             bar_left_x, pm_bar_y,
             bar_left_x + pm_w, pm_bar_y + bar_h,
             makecol(0, 0, 200));

    // Barre PA (rouge)
    int maxPA = classe.mana * 10;
    float pa_ratio = maxPA > 0
                     ? (float)perso->p_attaque / (float)maxPA
                     : 0.0f;
    if (pa_ratio < 0.0f) pa_ratio = 0.0f;
    if (pa_ratio > 1.0f) pa_ratio = 1.0f;
    int pa_w = (int)(bar_total_w * pa_ratio + 0.5f);
    rectfill(buffer,
             bar_left_x, pa_bar_y,
             bar_left_x + pa_w, pa_bar_y + bar_h,
             makecol(200, 0, 0));

    // 5) CADRE DE SÉLECTION DE COMPÉTENCE
    draw_sprite(buffer, classe.sprite[8], x0 - 4, y0 + 11);
    show_selected_comp(buffer, selected_competence);
}

void afficher_pv_joueurs(BITMAP* buffer, Game game) {
    const int start_x       = 10;
    const int start_y       = 10;
    const int line_height   = 25;
    const int bar_width_max = 150;
    const int bar_height    = 15;

    for (int i = 0; i < NB_JOUEURS; i++) {
        Perso* p = &game.players[i];

        // Affichage du pseudo
        textprintf_ex(buffer, font,
                      start_x, start_y + i * line_height,
                      makecol(255, 255, 255), -1,
                      "%s", p->pseudo);

        // calcul des PV et ratio
        int maxPV = p->classe.pv * 10;
        int curPV = p->pv_actuels;

        // clamp entre 0 et maxPV
        if (curPV < 0)     curPV = 0;
        if (curPV > maxPV) curPV = maxPV;

        // ratio [0..1]
        float pv_ratio = (maxPV > 0)
                        ? (float)curPV / (float)maxPV
                        : 0.0f;

        // largeur en pixels (avec arrondi)
        int pv_w = (int)(bar_width_max * pv_ratio + 0.5f);

        // dessin de la barre rouge (fond)
        rectfill(buffer,
                 start_x + 120,
                 start_y + i * line_height,
                 start_x + 120 + bar_width_max,
                 start_y + i * line_height + bar_height,
                 makecol(100, 0, 0));

        // dessin de la portion verte (PV restants)
        if (pv_w > 0) {
            rectfill(buffer,
                     start_x + 120,
                     start_y + i * line_height,
                     start_x + 120 + pv_w,
                     start_y + i * line_height + bar_height,
                     makecol(0, 200, 0));
        }

        // cadre blanc
        rect(buffer,
             start_x + 120,
             start_y + i * line_height,
             start_x + 120 + bar_width_max,
             start_y + i * line_height + bar_height,
             makecol(255, 255, 255));

        // Affichage texte PV
        textprintf_ex(buffer, font,
                      start_x + 280, start_y + i * line_height,
                      makecol(255, 255, 0), -1,
                      "%d / %d", curPV, maxPV);
    }
}

void bouton_next(BITMAP* buffer, BITMAP* icon) {
    if (!icon) return;
    const int pad = 10;
    int x = SCREEN_W - icon->w - pad;
    int y = SCREEN_H - icon->h - pad;
    draw_sprite(buffer, icon, x, y);
}


void show_graphique(Game game, int n_turns, int p_idx, BITMAP* buffer, BITMAP* curseur, BITMAP* panneau_bas_gauche, BITMAP* next_button, int selected_competence, time_t turn_start)
{
    // 1) Fond
    if (game.map.background) {
        stretch_blit(game.map.background, buffer,
                     0, 0,
                     game.map.background->w, game.map.background->h,
                     0, 0,
                     SCREEN_W, SCREEN_H);
    }

    // 2) Tuiles isométriques & personnages
    const int origin_x = SCREEN_W / 2;
    const int offset_y = SCREEN_H / 2 - TILE_HEIGHT * PLAT_Y / 2;

    for (int y = 0; y < PLAT_Y; y++) {
        for (int x = 0; x < PLAT_X; x++) {
            int id = game.plateau[x][y];
            int iso_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
            int iso_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;

            // Si la tuile est hors de l'écran, on passe
            if (iso_x + TILE_WIDTH <= 0 || iso_x >= SCREEN_W ||
                iso_y + TILE_HEIGHT <= 0 || iso_y >= SCREEN_H) {
                continue;
            }

            if (id < TILE_COUNT) {
                // Case normale (sol, herbe, eau, etc.)
                draw_sprite(buffer, game.map.images[id], iso_x, iso_y);
                afficher_portee(buffer, game, game.players[p_idx], x, y, iso_x, iso_y);
            }
            else {
                // Case avec joueur
                draw_sprite(buffer, game.map.images[0], iso_x, iso_y);
                afficher_portee(buffer, game, game.players[p_idx], x, y, iso_x, iso_y);

                Perso *perso = &game.players[id - TILE_COUNT];
                draw_sprite(buffer,
                            perso->classe.sprite[0],
                            iso_x,
                            iso_y - perso->classe.sprite[0]->h / 3);
            }

            /*
            // Affichage des sprites de mort (code non utilisé actuellement)
            for (int i = 0; i < NB_JOUEURS; i++) {
                Perso *pl = &game.players[i];
                if (pl->pv_actuels <= 0) {
                    draw_sprite(buffer, sprite_mort, iso_x, iso_y - sprite_mort->h / 2);
                }
            }
            */
        }
    }
    if (REPLAY==0) {
        // 3) Interface utilisateur (UI)
        barre_jeu(buffer,
              panneau_bas_gauche,
              game.players[p_idx].classe,
              &game.players[p_idx],
              selected_competence);
        show_selected_comp(buffer, selected_competence);
        bouton_next(buffer, next_button);

        // Barre de temps (blanche)
        float longueur = 1.0f - difftime(time(NULL), turn_start) / 15.0f;
        if (longueur > 0.0f) {
            rectfill(buffer, 0, SCREEN_H - 10, SCREEN_W * longueur, SCREEN_H, makecol(255, 255, 255));
        }

        // 4) Curseur
        stretch_sprite(buffer, curseur, mouse_x, mouse_y, 32, 32);
    }
    // Affichage des PV des joueurs en haut à gauche
    afficher_pv_joueurs(buffer, game);

    // 5) Envoi du buffer à l'écran
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
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

        translation_to_iso(&y, &x);
        if (x != -1 && y != -1) {
            action(game, &game->players[i], *competence, x, y, i);
            printf("%d : %d , %d\n",i, game->players[i].x, game->players[i].y);
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
                //process_data(game, i, buffer); // on traite les données des autres joueurs
                show(*game,n_turns,num);
            }
        }
    }
}


void jouer_graphique(socket_t sock, Game * game, int num) {
    game->nb_morts=0;
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    import_terrainJeu_Via_Fichier_texte(game);


    // Charger l'image du curseur
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }
    appliquer_transparence_curseur(curseur);
    // redimensionnement du curseur

    BITMAP* panneau_bas_gauche = charger_et_traiter_image(
        "../Projet/Graphismes/Interface/BarreDeJeu/1.bmp",
        1024*0.7, 459*0.7
    );
    BITMAP* next_button = charger_et_traiter_image(
        "../Projet/Graphismes/Menus/Boutons/NEXT.bmp",
        651*0.5, 342*0.5
    );

    long int received;
    char LAN_buffer[BUFFER_SIZE] = {0};
    int quit = 0;
    int n_turns = 0;
    int next = 0;
    int selected_competence=-1;
    show(*game, 0, num);
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            selected_competence=-1;
            init_portee(game);
            init_tour(game);
            time_t turn_start = time(NULL);
            next = 0;
            if (num == i) {
                while (!next) {
                    show_graphique(*game, n_turns, i, buffer, curseur, panneau_bas_gauche, next_button, selected_competence,turn_start);
                    tour_graphique(game, i, &selected_competence, &next, &quit);

                    // Vérification du timeout de 15 secondes
                    if (difftime(time(NULL), turn_start) >= 15.0) {
                        next = 1;
                    }
                    rest(10);

                    if (game->last_action[0]!='0') {
                        next=1;
                    }
                }
                game->last_action[strlen(game->last_action)] = '\0';
                printf("sending END : %s\n", game->last_action);
                send(sock, game->last_action, strlen(game->last_action), 0);

            } else {
                REPLAY =1;
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence,turn_start); // affiche l'ecrant de jeu
                printf("waiting for data\n");
                get_data(sock, &received, LAN_buffer,i, &quit);
                if(quit) break;
                process_data(game, i, LAN_buffer, &next);
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence,turn_start); // affiche l'ecrant de jeu
                REPLAY =0;
            }
            if (game->nb_morts==NB_JOUEURS-1) {
                quit =1;
                game->poduim[game->nb_morts]= game->players[i];
                game->nb_morts++;
            }
            if (quit) break;
        }
    }
    destroy_bitmap(panneau_bas_gauche);
    destroy_bitmap(buffer);
    destroy_bitmap(game->map.background);
    clear_keybuf();
}

void jouer_local(Game * game) {
    int quit = 0;
    int n_turns = 0;
    show(*game, 0, 0);
    while (!quit) {
        for (int i=0; i<NB_JOUEURS; i++) {
            n_turns++;
            tour(game, i, NULL );
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
    game->nb_morts=0;
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    import_terrainJeu_Via_Fichier_texte(game);

    // Charger et traiter le curseu
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }
    appliquer_transparence_curseur(curseur);
    // redimensionnement du curseur

    BITMAP* panneau_bas_gauche = charger_et_traiter_image(
        "../Projet/Graphismes/Interface/BarreDeJeu/1.bmp",
        1024*0.7, 459*0.7
    );
    BITMAP* next_button = charger_et_traiter_image(
        "../Projet/Graphismes/Menus/Boutons/NEXT.bmp",
        651*0.5, 342*0.5
    );

    int quit = 0;
    int next = 0;
    int n_turns = 0;
    int selected_competence = -1;

    while (!quit) {
        for (int i = 0; i < NB_JOUEURS; i++) {
            n_turns++;
            selected_competence = -1;
            next = 0;
            init_tour(game);
            init_portee(game);
            if (game->players[i].pv_actuels == 0) next = 1;
            // Début du chronométrage du tour
            time_t turn_start = time(NULL);

            while (!next) {
                show_graphique(*game, n_turns, i, buffer, curseur, panneau_bas_gauche, next_button, selected_competence,turn_start);
                tour_graphique(game, i, &selected_competence, &next, &quit);

                // Vérification du timeout de 15 secondes
                if (difftime(time(NULL), turn_start) >= 15.0) {
                    next = 1;
                }

                rest(10);
            }
            if (game->nb_morts==NB_JOUEURS-1) {
                quit =1;
                game->poduim[game->nb_morts]= game->players[i];
                game->nb_morts++;
            }
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
