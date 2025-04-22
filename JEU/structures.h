#ifndef STRUCTURES_H
#define STRUCTURES_H

#define X 10
#define Y 10

typedef struct {
    // partie imuable ( envoyée dans initGame )
    char name[20];


    // pertie changeante ( envoyée à chaque tour )
    int x,y;

}Perso;

typedef struct {
    int plateau[X][Y];
    Perso players[4];
}Game;


#endif //STRUCTURES_H
