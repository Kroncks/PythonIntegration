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

void attack_statut(Perso* self, int idx) {
    // Sécurité : idx dans [0..3]
    if (idx < 0 || idx >= 4) return;

    const char* nom = self->classe.competences[idx].nom_competence;

    // Compétences de protection
    if (strcmp(nom, "Mur de lianes") == 0 ||
        strcmp(nom, "Bulle d'eau") == 0) {
        self->protection = true;
        return;
        }

    // Compétences de soin
    if (strcmp(nom, "Aromatherapie") == 0 ||
        strcmp(nom, "Recolte")       == 0 ||
        strcmp(nom, "Eau de vie")    == 0) {
        int soin = self->classe.competences[idx].degat * self->classe.foi;
        self->pv_actuels += soin;
        if (self->pv_actuels > self->classe.pv)
            self->pv_actuels = self->classe.pv;
        return;
        }

    // Compétence de boost
    if (strcmp(nom, "Rage") == 0) {
        self->boost_modifier = 1.5f;
    }
    self->p_attaque -= self->classe.competences[idx].p_attaque;
    return;
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

    // --- Sort de statut : animation sur soi ---
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

    // --- Attaque classique : animation sur la cible ---
    int iso_x = (defenseur->x - defenseur->y)*(TILE_WIDTH/2) + origin_x;
    int iso_y = (defenseur->x + defenseur->y)*(TILE_HEIGHT/2) + offset_y;
    for (int f = 0; f < 3; f++) {
        draw_sprite(screen,
                    spell->sprite[f],
                    iso_x,
                    iso_y - spell->sprite[f]->h/2);
        rest(100);
    }

    // 5) Sinon on vérifie la portée et on calcule les dégâts
    int dx = abs(attaquant->x - defenseur->x);
    int dy = abs(attaquant->y - defenseur->y);
    //if (dx + dy > spell->portee)
        //return;

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

    // 5) Consommation PA et application dégâts
    attaquant->p_attaque  -= spell->p_attaque;
    defenseur->pv_actuels -= dmg;
    if (defenseur->pv_actuels <= 0) {
        defenseur->pv_actuels = 0;
        game->poduim[game->nb_morts] = *defenseur;
        game->nb_morts+=1;
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

void inversion_chemin(Node map_path[PLAT_Y][PLAT_X], int len, Node path[len], int dest_x, int dest_y, int origin_x, int origin_y) {

    int i=0;
    int cx = dest_x, cy = dest_y;
    path[0].x = origin_x;
    path[0].y = origin_y;
    path[len].x = cx;
    path[len].y = cy;
    printf("fin init inv\n");
    while (!(cx == origin_x && cy == origin_y)) {
        printf("ahhh\n");
        Node p2 = map_path[cy][cx];
        cx = p2.x;
        cy = p2.y;
        i++;
        path[len-i-1]=p2;
        printf("aled\n");
    }
    for (int j = 0; j < len+1; j++) {
        printf("%d , %d\n", path[j].x, path[j].y);
    }
}

bool can_move(Game game, const Perso self, const int x_dest, const int y_dest,Node map_path[PLAT_Y][PLAT_X],int* len_path) {
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
    if (!verif_bfs(game, self.x, self.y, x_dest, y_dest, self.pm_restant,map_path,len_path)) return false;
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
    // 1) Origine logique
    int origin_x = self->x;
    int origin_y = self->y;

    // 2) Back-buffer
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) buffer = screen;

    // 3) Reconstruction du chemin
    int steps = len_path;
    Node path[steps+1];
    inversion_chemin(map_path, steps, path,
                     x_dest, y_dest,
                     origin_x, origin_y);

    // 4) Origine écran isométrique
    const int originScrX = SCREEN_W/2;
    const int originScrY = SCREEN_H/2 - TILE_HEIGHT*PLAT_Y/2;

    // 5) Parcours des segments
    for (int s = 0; s < steps; s++) {
        // a) Redessiner MAP + AUTRES joueurs
        if (game->map.background) {
            stretch_blit(game->map.background, buffer,
                         0,0,
                         game->map.background->w, game->map.background->h,
                         0,0,
                         SCREEN_W, SCREEN_H);
        }

        for (int y = 0; y < PLAT_Y; y++) {
            for (int x = 0; x < PLAT_X; x++) {
                int id = game->plateau[x][y];
                int iso_x = (x - y)*(TILE_WIDTH/2) + originScrX;
                int iso_y = (x + y)*(TILE_HEIGHT/2) + originScrY;

                if (iso_x + TILE_WIDTH  <= 0 || iso_x >= SCREEN_W ||
                    iso_y + TILE_HEIGHT <= 0 || iso_y >= SCREEN_H)
                    continue;

                // tuile
                draw_sprite(buffer,
                            game->map.images[id < TILE_COUNT ? id : 0],
                            iso_x, iso_y);

                // si case occupée et pas le self
                if (id >= TILE_COUNT) {
                    int pidx = id - TILE_COUNT;
                    if (pidx != num_joueur) {
                        Perso* pl = &game->players[pidx];
                        BITMAP* spr = (pl->pv_actuels > 0)
                            ? pl->classe.sprite[0]
                            : sprite_mort;
                        int oy = (spr == sprite_mort)
                                 ? spr->h/2
                                 : spr->h/3;
                        draw_sprite(buffer,
                                    spr,
                                    iso_x,
                                    iso_y - oy);
                    }
                }
            }
        }

        // b) Choix des frames selon la direction
        int dx = path[s+1].x - path[s].x;
        int dy = path[s+1].y - path[s].y;
        int f0, f1;
        if      (dx > 0) { f0 = 4; f1 = 5; }  // →
        else if (dx < 0) { f0 = 2; f1 = 3; }  // ←
        else if (dy > 0) { f0 = 4; f1 = 5; }  // ↓
        else             { f0 = 6; f1 = 7; }  // ↑

        // c) Animation 2 frames du self
        for (int frame = 0; frame < 2; frame++) {
            // (re)dessiner la frame courante du self
            int px = path[s].x, py = path[s].y;
            int iso_x = (px - py)*(TILE_WIDTH/2) + originScrX;
            int iso_y = (px + py)*(TILE_HEIGHT/2) + originScrY;

            BITMAP* spr = self->classe.sprite[
                (frame == 0) ? f0 : f1
            ];
            draw_sprite(buffer,
                        spr,
                        iso_x,
                        iso_y - spr->h/3);

            // blit & pause
            blit(buffer, screen,
                 0,0,
                 0,0,
                 SCREEN_W, SCREEN_H);
            rest(50);
        }

        // d) Mise à jour de la position
        self->x = path[s+1].x;
        self->y = path[s+1].y;
    }

    // 6) Position finale
    self->x = x_dest;
    self->y = y_dest;
    self->pm_restant -= steps;

    // 7) Mettre à jour la map
    game->plateau[origin_x][origin_y] = 0;
    game->plateau[x_dest][y_dest]     = TILE_COUNT + num_joueur;

    // 8) Libérer le buffer
    if (buffer != screen) destroy_bitmap(buffer);
}

void action(Game* game,
            Perso* self,
            const int num_competence,   // 1-based: 1–4 = sorts, 5 = déplacement
            const int action_x,
            const int action_y,
            int num_joueur)
{
    // Valider num_competence
    if (num_competence < 1 || num_competence > 5)
        return;

    printf("log action: compétence %d, cible (%d,%d)\n",
           num_competence, action_x, action_y);

    if (num_competence == 5) {
        // déplacement
        int len_path;
        Node map_path[PLAT_Y][PLAT_X];
        // initialiser map_path à -1
        for (int i = 0; i < PLAT_Y; i++) {
            for (int j = 0; j < PLAT_X; j++) {
                map_path[i][j].x = -1;
                map_path[i][j].y = -1;
            }
        }

        printf("Début can_move\n");
        if (can_move(*game, *self, action_x, action_y,
                     map_path, &len_path))
        {
            printf("Début deplacement\n");
            deplacement(game, self,
                        action_x, action_y,
                        map_path, len_path,
                        num_joueur);
            // TODO : envoyer déplacement over network
        }
    }
    else {
        // attaque ou sort : convertir 1-based → 0-based
        int idx = num_competence - 1;
        // trouver l’indice du joueur ciblé
        int cible = found_player(*game, action_x, action_y);
        if (cible >= 0 && cible < NB_JOUEURS) {
            attack(game, self, &game->players[cible], idx);
            printf("DEBUG: PV du joueur %d (cible) = %d\n",
               cible,
               game->players[cible].pv_actuels);
            // **Nouvel appel** : vérifier et afficher la mort si PV ≤ 0
            // TODO : envoyer attaque over network
        }
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
Coord prev[PLAT_Y][PLAT_X]; // pour reconstruire le chemin


void dishtra(Game *game, int start_x, int start_y, int portee, int pas) {
    typedef struct {
        int x, y, dist;
    } Node;

    Node queue[PLAT_X * PLAT_Y];
    int front = 0, rear = 0;

    game->portee[start_y][start_x] = 1;
    game->prev[start_y][start_x].x = -1;
    game->prev[start_y][start_x].y = -1;

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
            if (game->plateau[ny][nx] != 0) continue;
            if (game->portee[ny][nx]) continue;

            game->portee[ny][nx] = 1;
            game->prev[ny][nx].x = curr.x;
            game->prev[ny][nx].y = curr.y;

            queue[rear++] = (Node){nx, ny, curr.dist + 1};
        }
    }
}

// Stocke le chemin de (x, y) jusqu'au joueur dans path[], retourne sa taille
int get_path(Game *game, int x, int y, Coord path[], int max_len) {
    int len = 0;
    while (x != -1 && y != -1 && len < max_len) {
        path[len++] = (Coord){x, y};
        Coord p = game->prev[y][x];
        x = p.x;
        y = p.y;
    }

    // Inverser le chemin pour aller du joueur vers la destination
    for (int i = 0; i < len / 2; i++) {
        Coord tmp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = tmp;
    }

    return len;
}


void update_portee(Game * game, Perso player, int num_competence) {
    init_portee(game);
    int portee;
    if (num_competence == 5 )
        portee = player.pm_restant;
    else
        portee = player.classe.competences[num_competence-1].portee;

    printf("portee : %d\n", portee);

    int x = player.y;
    int y = player.x;
    int pas =0;
    dishtra(game, x, y, portee, pas);


    for (int i = 0; i < PLAT_X; i++) {
        for (int j = 0; j < PLAT_Y; j++) {
            printf("%d  ", game->portee[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < PLAT_X; i++) {
        for (int j = 0; j < PLAT_Y; j++) {
            printf("%d  ", game->plateau[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
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


void show_graphique(Game game,
                    int n_turns,
                    int p_idx,             // index du joueur courant (pour l'UI)
                    BITMAP* buffer,
                    BITMAP* curseur,
                    BITMAP* panneau_bas_gauche,
                    BITMAP* next_button,
                    int selected_competence,
                    time_t turn_start)
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
    const int origin_x = SCREEN_W/2;
    const int offset_y = SCREEN_H/2 - TILE_HEIGHT * PLAT_Y / 2;

    for (int y = 0; y < PLAT_Y; y++) {
        for (int x = 0; x < PLAT_X; x++) {
            int id = game.plateau[x][y];
            int iso_x = (x - y) * (TILE_WIDTH/2) + origin_x;
            int iso_y = (x + y) * (TILE_HEIGHT/2) + offset_y;

            // Si hors écran, on skip
            if (iso_x + TILE_WIDTH <= 0 || iso_x >= SCREEN_W
             || iso_y + TILE_HEIGHT <= 0 || iso_y >= SCREEN_H) {
                continue;
            }

            if (id < TILE_COUNT) {
                // Case vide
                draw_sprite(buffer,
                            game.map.images[id],
                            iso_x,
                            iso_y);
                afficher_portee(buffer, game,
                                game.players[p_idx],
                                x, y,
                                iso_x, iso_y);
            } else {
                int iso_x = (x - y) * (TILE_WIDTH / 2) + origin_x;
                int iso_y = (x + y) * (TILE_HEIGHT / 2) + offset_y;
                if (iso_x + TILE_WIDTH > 0 && iso_x < SCREEN_W && iso_y + TILE_HEIGHT > 0 && iso_y < SCREEN_H) {
                    draw_sprite(buffer, game.map.images[0],iso_x, iso_y);
                    afficher_portee(buffer,game, game.players[p_idx], x, y, iso_x,iso_y);
                    draw_sprite(buffer, game.players[id-TILE_COUNT].classe.sprite[0],iso_x, iso_y-game.players[id-TILE_COUNT].classe.sprite[0]->h/3);
                    }
            }

                for (int i=0; i<NB_JOUEURS;i++)
                {
                    Perso *pl = &game.players[i];

                    if (pl->pv_actuels <= 0) {
                        game.plateau[pl->x][pl->y] -= TILE_COUNT - i+1;

                        draw_sprite(buffer,
                                    sprite_mort,
                                    iso_x,
                                    iso_y - sprite_mort->h/2);
                    }
                }

            }
        }


    // 3) UI en bas à gauche
    barre_jeu(buffer,
              panneau_bas_gauche,
              game.players[p_idx].classe,
              selected_competence);
    show_selected_comp(buffer, selected_competence);
    bouton_next(buffer, next_button);
    // Barre de temps
    float longueur = 1-difftime(time(NULL), turn_start)/15.0;
    rectfill(buffer, 0, SCREEN_H, SCREEN_W*longueur, SCREEN_H-10, makecol(255, 255, 255));

    // 4) Curseur
    stretch_sprite(buffer,
                   curseur,
                   mouse_x, mouse_y,
                   32, 32);

    // 5) Envoi à l'écran
    blit(buffer, screen,
         0, 0,
         0, 0,
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
    game->nb_morts=0;
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

    if (!sprite_mort) {
        sprite_mort = load_bitmap("../Projet/Graphismes/Animations/Mort/1.bmp", NULL);
        if (!sprite_mort) {
            allegro_message("Erreur : impossible de charger le sprite de mort !");
            exit(EXIT_FAILURE);
        }
    }


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
            time_t turn_start = time(NULL);
            if (num == i) {
                while (!next) {
                    show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche, next_button, selected_competence,turn_start); // affiche l'ecrant de jeu
                    //tour_graphique(game, i, &next ); // verifie les actions du joueur et joue joue
                    rest(10);
                }
                tour(game, i, LAN_buffer); // le joueur joue
                send(sock, LAN_buffer, strlen(LAN_buffer), 0); // les données sont envoyées
                printf("[Game] Data sent\n");
            } else {
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence,turn_start); // affiche l'ecrant de jeu
                get_data(sock, &received, LAN_buffer,i, &quit); // on attends de recevoir les données
                if(quit) break;
                process_data(game, i, LAN_buffer); // on traite les données des autres joueurs
                show_graphique(*game,n_turns,i, buffer, curseur,panneau_bas_gauche,next_button, selected_competence,turn_start); // affiche l'ecrant de jeu
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
    game->nb_morts=0;
    BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        allegro_message("Erreur lors de la création du buffer !");
        exit(EXIT_FAILURE);
    }

    import_terrainJeu_Via_Fichier_texte(game);

    // Charger et traiter le curseur…
    curseur = load_bitmap("../DATA/curseur.bmp", NULL);
    if (!curseur) {
        allegro_message("Impossible de charger l'image du curseur !");
        exit(EXIT_FAILURE);
    }
    appliquer_transparence_curseur(curseur);
    // … redimensionnement du curseur …

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
            init_portee(game);

            // ----- Début du chronométrage du tour -----
            time_t turn_start = time(NULL);

            while (!next) {
                show_graphique(*game, n_turns, i, buffer, curseur, panneau_bas_gauche, next_button, selected_competence,turn_start);
                tour_graphique(game, i, &selected_competence, &next, &quit);

                // Vérification du timeout de 15 secondes
                if (difftime(time(NULL), turn_start) >= 15.0) {
                    next = 1;  // Force la fin du tour
                }

                rest(10);
            }
            // ----- Fin du chronométrage du tour -----

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
