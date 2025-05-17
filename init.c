#include "init.h"

t_classe classes_disponibles[12];
BITMAP* avatars[12][8];


void init_player_classe( Perso * self) {

    int num_classe = self->avatar[0] - 'a';
    //Chargement de la classe
    self->classe = classes_disponibles[num_classe];
    //Boost ? Protection ?
    self->protection = false;
    self->boost_modifier = 1.0;
    //Initialisation des variables ingame
    self->pm_restant = self->classe.endurance*10;
    self->pv_actuels = self->classe.pv*10;
    self->p_attaque = self->classe.mana*10;



}


void import_animations() {
    for (int i; i<12; i++) {
        for (int j; j<8; j++) {
            avatars[i][j] = load_bitmap("../Projet/Sprites/avatar/avatar" + (char)(i + 'a') + ".bmp", NULL);
        }
    }
}