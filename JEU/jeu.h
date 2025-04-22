#ifndef JEU_H
#define JEU_H

#include "../LAN/LAN.h"
#include "structures.h"

void init_game(socket_t sock, Game game, int num, char * username);
void jouer(socket_t sock, Game game, int num);

#endif //JEU_H
