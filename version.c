#include "version.h"

#define LEN 20
#define FILENAME "../version.txt"

void version() {
    char version_n[LEN];
    FILE *file = fopen(FILENAME, "r");
    if (fgets(version_n, LEN, file) != NULL) {
        // Enlève le saut de ligne s'il y en a un
        version_n[strcspn(version_n, "\n")] = '\0';
        printf("Version : %s\n", version_n);
    } else {
        printf("Erreur lors de la lecture de la version.\n");
    }
    fclose(file);
}
