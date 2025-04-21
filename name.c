#include "name.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Fonction pour vérifier si la chaîne contient uniquement des lettres et chiffres
int blind(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isalnum((unsigned char)str[i])) {
            return 0; // Caractère non valide
        }
    }
    return 1; // Tout est valide
}

void name(char * username) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        printf("Entrez un pseudo (lettres et chiffres uniquement) : ");
        if (scanf("%99s", username) != 1) {
            fprintf(stderr, "Erreur de lecture.\n");
        }

        // Vérification du pseudo
        if (!blind(username)) {
            printf("Le pseudo contient des caractères invalides.\n");
        }

        // Écriture dans le fichier
        file = fopen(FILENAME, "w");
        if (file == NULL) {
            perror("Erreur lors de l'ouverture du fichier en écriture");
        }

        fprintf(file, "%s", username);
        fclose(file);

        printf("Pseudo enregistré avec succès.\n");
    } else {
        if (fgets(username, MAX_USERNAME, file) != NULL) {
            // Enlève le saut de ligne s'il y en a un
            username[strcspn(username, "\n")] = '\0';
            printf("Bonjour %s\n", username);
        } else {
            printf("Erreur lors de la lecture du pseudo.\n");
        }

        fclose(file);
    }
}
