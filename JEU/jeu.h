#ifndef JEU_H
#define JEU_H

#include "../GRAPHISMES/graphismes.h"
#include "../LAN/LAN.h"
#include "structures.h"
#include "../GRAPHISMES/bouttons.h"
#include "../init.h"
#include "../menu.h"

int NB_JOUEURS;

Perso init_player(int num);
void init_game(socket_t sock, Game * game, int num, Perso self);
void jouer(socket_t sock, Game * game, int num);
void init_local_game(Game * game, Perso * liste);
void jouer_local(Game * game);
void init_nb_players();
void jouer_local_graphique(Game * game);
void jouer_graphique(socket_t sock, Game * game, int num);
static void barre_joueur(BITMAP* buffer, BITMAP* icon);
int get_path(Game *game, int x, int y, Coord path[], int max_len) ;
void dishtra(Game *game, int start_x, int start_y, int portee, int pas);
void bouton_next(BITMAP* buffer, BITMAP* icon);
void next_cliqued(int * next);
int change_music(const char *filename);

#endif //JEU_H
