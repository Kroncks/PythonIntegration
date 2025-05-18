#include "init.h"


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

