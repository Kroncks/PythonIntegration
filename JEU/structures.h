#ifndef STRUCTURES_H
#define STRUCTURES_H

#define PLAT_X 20
#define PLAT_Y 20
#define MAX_NODE PLAT_X*PLAT_Y
#include <math.h>
#include <allegro.h>
#include <string.h>
#include <stdbool.h>
#define TILE_COUNT 3

typedef struct {
    int x;
    int y;
    int precedent_x;
    int precedent_y;
}Node;

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
    BITMAP* sprite[9]; //4 sprites de déplacement et un de menu
}t_classe;


typedef struct {
    BITMAP* images[TILE_COUNT];
    BITMAP* background;
} Map;


typedef struct {
    //Partie imuable statique (on reste sur le meme pc)
    t_classe classe;
    // partie imuable ( envoyée dans initGame )
    char pseudo[20];
    char avatar[2];

    // partie changeante ( envoyée à chaque tour )
    int x,y;
    //Partie changeante (non envoyée à chaque tour)
    int pm_restant;
    int p_attaque;
    int pv_actuels;
    float boost_modifier; //à initialiser à 1.0
    bool protection;

}Perso;

typedef struct {
    char name[20];
} Client;

typedef struct {
    int x, y;
} Coord;



typedef struct {
    int plateau[PLAT_X][PLAT_Y];
    int portee[PLAT_X][PLAT_Y];
    Coord prev[PLAT_X][PLAT_Y];
    Perso players[4];
    Client client;
    Map map;
    int theme;
}Game;


#endif //STRUCTURES_H
