#ifndef BOUTTONS_H
#define BOUTTONS_H

#include <allegro.h>

typedef struct {
    int x, y;
    int w, h;
} Rect;

typedef struct {
    BITMAP* image;
    Rect rect;
} Bouton;

void init_boutons(Bouton* boutons, const char** chemins, int nb);
void afficher_boutons(BITMAP* buffer, Bouton* boutons, int nb);
int bouton_clique(Bouton* boutons, int nb, int x, int y);
void detruire_boutons(Bouton* boutons, int nb);

#endif // BOUTTONS_H
