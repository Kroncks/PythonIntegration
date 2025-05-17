#ifndef STRUCTURES_H
#define STRUCTURES_H

#define PLAT_X 20
#define PLAT_Y 20

#include <allegro.h>

#define TILE_COUNT 3

typedef struct {
    char* nom_competence;
    int ID_competence;
    int degat;
    char type_degat; //C(contandant), T(tranchant), P(percant), E(eau), F(feu), S(sol/terre), N(neutre/statut)
    char type_stat; //F(faith), S(strength), I(intelligence), D(dexterity)
    int portee;
    int p_attaque;
    BITMAP* sprite[3]; //3 frames par compétences
}t_competence;

typedef struct {
    t_competence competences[4];
    int pv;
    int mana;
    int endurance;
    int force;
    int dexterite;
    int intelligence;
    int foi;
    float r_contandant;
    float r_tranchant;
    float r_percant;
    float r_eau;
    float r_feu;
    float r_terre;
    BITMAP* sprite[8]; //4 sprites de déplacement et un de menu
}t_classe;


typedef struct {
    BITMAP* images[TILE_COUNT];
    BITMAP* background;
} Map;


typedef struct {
    // partie imuable ( envoyée dans initGame )
    char pseudo[20];
    char avatar[2];

    // partie changeante ( envoyée à chaque tour )
    int x,y;

}Perso;

typedef struct {
    char name[20];
} Client;

typedef struct {
    int plateau[PLAT_X][PLAT_Y];
    Perso players[4];
    Client client;
    Map map;
    int theme;
}Game;


#endif //STRUCTURES_H
