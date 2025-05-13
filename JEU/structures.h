#ifndef STRUCTURES_H
#define STRUCTURES_H

#define PLAT_X 10
#define PLAT_Y 10

typedef struct {
    // partie imuable ( envoyée dans initGame )
    char pseudo[20];
    char avatar[2];

    // pertie changeante ( envoyée à chaque tour )
    int x,y;

}Perso;

typedef struct {
    char name[20];
} Client;

typedef struct {
    int plateau[PLAT_X][PLAT_Y];
    Perso players[4];
    Client client;
}Game;


#endif //STRUCTURES_H
