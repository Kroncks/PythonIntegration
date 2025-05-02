#ifndef JEU_H
#define JEU_H

#include "../LAN/LAN.h"
#include "structures.h"

int NB_JOUEURS;

Perso init_player();
void init_game(socket_t sock, Game * game, int num, Perso self);
void jouer(socket_t sock, Game * game, int num);
void init_local_game(Game * game, Perso * liste);
void jouer_local(Game * game);

#endif //JEU_H
