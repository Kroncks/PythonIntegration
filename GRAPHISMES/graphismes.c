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


BITMAP* redimensionner_image(BITMAP *source, int new_width, int new_height) {
    BITMAP *resized = create_bitmap(new_width, new_height);
    if (!resized) {
        allegro_message("Erreur lors de la création de l'image redimensionnée !");
        return NULL;
    }

    clear_to_color(resized, makecol(255, 0, 255));
    stretch_blit(source, resized, 0, 0, source->w, source->h, 0, 0, new_width, new_height);

    return resized;
}



BITMAP* traiter_image(BITMAP *source) {
    BITMAP *backless = create_bitmap(source->w, source->h);
    if (!backless) {
        allegro_message("Erreur lors de la création de l'image sans fond !");
        exit(1);
    }

    clear_to_color(backless, makecol(255, 0, 255));

    for (int i = 0; i < source->w; i++) {
        for (int j = 0; j < source->h; j++) {
            int pixel = getpixel(source, i, j);
            int r = getr(pixel);
            int g = getg(pixel);
            int b = getb(pixel);

            if ((r >= 180 && r <= 255) && (g >= 0 && g <= 80) && (b >= 180 && b <= 255)) {
                putpixel(backless, i, j, makecol(255, 0, 255));
            } else {
                putpixel(backless, i, j, pixel);
            }
        }
    }

    return backless;
}

BITMAP* charger_et_traiter_image(const char *path, int new_width, int new_height) {
    BITMAP *source = load_bitmap(path, NULL);
    if (!source) {
        allegro_message("Erreur lors du chargement de l'image !");
        exit(EXIT_FAILURE);
    }

    BITMAP *image_sans_fond = traiter_image(source);
    destroy_bitmap(source);

    BITMAP *image_redimensionnee = redimensionner_image(image_sans_fond, new_width, new_height);
    destroy_bitmap(image_sans_fond);

    return image_redimensionnee;
}