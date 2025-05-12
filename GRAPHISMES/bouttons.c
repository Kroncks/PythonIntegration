#include "bouttons.h"
#include "../globals.h"

#define COLOR_TOLERANCE 100

int est_rose(int r, int g, int b) {
    return (abs(r - 255) <= COLOR_TOLERANCE &&
            abs(g - 0)   <= COLOR_TOLERANCE &&
            abs(b - 255) <= COLOR_TOLERANCE);
}

void appliquer_transparence(BITMAP* image) {
    for (int y = 0; y < image->h; y++) {
        for (int x = 0; x < image->w; x++) {
            int c = getpixel(image, x, y);
            int r = getr(c), g = getg(c), b = getb(c);
            if (est_rose(r, g, b)) {
                putpixel(image, x, y, makecol(255, 0, 255));  // couleur masque Allegro
            }
        }
    }
}

void init_boutons(Bouton* boutons, const char** chemins, int nb) {
    int total_height = 0;
    int espace = SCREEN_HEIGHT / 20;

    for (int i = 0; i < nb; i++) {
        boutons[i].image = load_bitmap(chemins[i], NULL);
        if (!boutons[i].image) {
            allegro_message("Erreur chargement bouton %d", i);
            exit(EXIT_FAILURE);
        }
        appliquer_transparence(boutons[i].image);

        // Échelle uniquement en hauteur
        int target_height = SCREEN_HEIGHT / 10;
        float ratio = (float)target_height / boutons[i].image->h;
        int scaled_width = boutons[i].image->w * ratio;

        boutons[i].rect.w = scaled_width;
        boutons[i].rect.h = target_height;

        total_height += target_height;
    }
    total_height += espace * (nb - 1);
    int start_y = (SCREEN_HEIGHT - total_height) / 2;

    for (int i = 0; i < nb; i++) {
        boutons[i].rect.x = (SCREEN_WIDTH - boutons[i].rect.w) / 2;
        boutons[i].rect.y = start_y;
        start_y += boutons[i].rect.h + espace;
    }
}

void afficher_boutons(BITMAP* buffer, Bouton* boutons, int nb) {
    for (int i = 0; i < nb; i++) {
        Rect r = boutons[i].rect;
        int survol = (mouse_x >= r.x && mouse_x <= r.x + r.w &&
                      mouse_y >= r.y && mouse_y <= r.y + r.h);

        if (survol) {
            // Agrandissement de 10%
            int w = r.w * 1.1;
            int h = r.h * 1.1;
            int dx = (w - r.w) / 2;
            int dy = (h - r.h) / 2;

            stretch_sprite(
                buffer,
                boutons[i].image,
                r.x - dx,
                r.y - dy,
                w,
                h
            );
        } else {
            stretch_sprite(
                buffer,
                boutons[i].image,
                r.x,
                r.y,
                r.w,
                r.h
            );
        }
    }
}


int bouton_clique(Bouton* boutons, int nb, int x, int y) {
    for (int i = 0; i < nb; i++) {
        Rect r = boutons[i].rect;
        if (x >= r.x && x <= r.x + r.w &&
            y >= r.y && y <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

void detruire_boutons(Bouton* boutons, int nb) {
    for (int i = 0; i < nb; i++) {
        destroy_bitmap(boutons[i].image);
    }
}
