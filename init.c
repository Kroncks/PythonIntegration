#include "init.h"

void transfert_temp_resistance(int r_temp, float* r_resistance) {
    if (r_temp==0) *r_resistance = 0.5;
    else if (r_temp==1) *r_resistance = 1;
    else if (r_temp==2) *r_resistance = 2;
}

t_classe init_classe(int num_classe) {
    t_classe classe;
    int lenght_nom [4];
    char filename[100];
    int temp;
    snprintf(filename, sizeof(filename),"../Projet/Fichiers textes/Classe/%d.txt", num_classe+1);
    printf("%s\n",filename);
    FILE* p_fichier_classe = fopen(filename, "r");
    if (p_fichier_classe == NULL) {
        printf("Erreur lors du chargement du fichier de classe");
        exit(EXIT_FAILURE);
    }

    //chargement des tailles des noms competences de la classe
    for (int j=0; j<4;j++) {
        fscanf(p_fichier_classe,"%d", &lenght_nom[j]);
    }
    printf("Taille des noms chargées\n");

    fscanf(p_fichier_classe,"\n");
    //Chargement des données de la classe
    fscanf(p_fichier_classe,"%d", &classe.pv);
    fscanf(p_fichier_classe,"%d", &classe.mana);
    fscanf(p_fichier_classe,"%d", &classe.endurance);
    fscanf(p_fichier_classe,"%d", &classe.force);
    fscanf(p_fichier_classe,"%d", &classe.dexterite);
    fscanf(p_fichier_classe,"%d", &classe.intelligence);
    fscanf(p_fichier_classe,"%d", &classe.foi);
    printf("Stat chargées\n");
    fscanf(p_fichier_classe,"\n");

    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_contandant);
    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_tranchant);
    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_percant);
    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_eau);
    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_feu);
    fscanf(p_fichier_classe,"%d", &temp);
    transfert_temp_resistance(temp, &classe.r_terre);
    printf("Resistance chargées\n");
    fscanf(p_fichier_classe,"\n");

    for (int i=0; i<4;i++) {
        char token;
        classe.competences[i].nom_competence = malloc(lenght_nom[i]+1*sizeof(char));
        fgets(classe.competences[i].nom_competence, lenght_nom[i]+1, p_fichier_classe);
        printf("Nom : %s\n",classe.competences[i].nom_competence);
        fscanf(p_fichier_classe,"%d", &classe.competences[i].ID_competence);
        fscanf(p_fichier_classe,"%d", &classe.competences[i].degat);
        fscanf(p_fichier_classe,"%d", &classe.competences[i].p_attaque);
        fscanf(p_fichier_classe,"%d", &classe.competences[i].portee);
        fscanf(p_fichier_classe, "%c", &token); //Liberation espace
        fscanf(p_fichier_classe,"%c", &classe.competences[i].type_degat);
        fscanf(p_fichier_classe, "%c", &token); //Liberation espace
        fscanf(p_fichier_classe,"%c", &classe.competences[i].type_stat);
        fscanf(p_fichier_classe,"\n");
        printf("Competence %d chargée\n",i+1);
    }
    fclose(p_fichier_classe);

    //Chargement des sprites de la classe

    for (int i=0; i<8; i++) {
        for (int i=0; i<8; i++) {
            snprintf(filename, sizeof(filename),"../Projet/Graphismes/Animations/Persos/%d/%d.bmp", num_classe+1,i+1);
            classe.sprite[i] = charger_et_traiter_image(filename, 64,64);
            if (classe.sprite[i]==NULL) {
                printf("\nPerso %d sprite %d introuvable", num_classe+1,i+1);
            }
        }
    }
    for (int i=0; i<4;i++) {
        for (int k=0;k<3;k++) {
            snprintf(filename, sizeof(filename),"../Projet/Graphismes/Animations/Competences/%d/%d/%d.bmp", num_classe+1,i+1,k+1);
            classe.competences[i].sprite[k] = charger_et_traiter_image(filename, 64,64);
        }
    }
    return classe;
}

void init_player_classe( Perso * self) {

    int num_classe = self->avatar[0] - 'a';
    //Chargement de la classe
    self->classe = init_classe(num_classe);
    //Boost ? Protection ?
    self->protection = false;
    self->boost_modifier = 1.0;
    //Initialisation des variables ingame
    self->pm_restant = self->classe.endurance;
    self->pv_actuels = self->classe.pv*10;
    self->p_attaque = self->classe.mana*10;
}

