#ifndef GRAPHISMES_H
#define GRAPHISMES_H

#include <allegro.h>


void appliquer_transparence_curseur(BITMAP* curseur);
int rose(int r, int g, int b);
BITMAP* charger_et_traiter_image(const char *path, int new_width, int new_height);


#endif //GRAPHISMES_H
