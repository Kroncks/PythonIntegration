#include "graphismes.h"

#define COLOR_TOLERANCE 100

int rose(int r, int g, int b) {
    return (abs(r - 255) <= COLOR_TOLERANCE &&
            abs(g - 0)   <= COLOR_TOLERANCE &&
            abs(b - 255) <= COLOR_TOLERANCE);
}



void appliquer_transparence_curseur(BITMAP* curseur) {
    for (int y = 0; y < curseur->h; y++) {
        for (int x = 0; x < curseur->w; x++) {
            int c = getpixel(curseur, x, y);
            int r = getr(c), g = getg(c), b = getb(c);
            if (rose(r, g, b)) {
                putpixel(curseur, x, y, makecol(255, 0, 255));  // Masque transparent
            }
        }
    }
}