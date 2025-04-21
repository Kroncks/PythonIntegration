#include "LAN.h"


void client(char * username) {
    char buffer[128];
    printf("[Client C] Lancement de la détection du serveur via client.py...\n");

    FILE *fp = popen("../.venv/bin/python3 ../LAN/client.py", "r");
    if (fp == NULL) {
        perror("[Client C] Erreur appel script Python");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        printf("[Client C] Échec de la détection du serveur (aucun retour de client.py).\n");
        pclose(fp);
        return;
    }
    pclose(fp);
    buffer[strcspn(buffer, "\n")] = 0;

    printf("[Client C] Serveur détecté à l'adresse : %s\n", buffer);

    // Connexion TCP
    printf("[Client C] Création de la socket...\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[Client C] Erreur création socket");
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    if (inet_pton(AF_INET, buffer, &serv_addr.sin_addr) <= 0) {
        perror("[Client C] Adresse invalide ou conversion échouée");
        close(sock);
        return;
    }

    printf("[Client C] Tentative de connexion au serveur...\n");
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[Client C] Connexion échouée");
        close(sock);
        return;
    }

    printf("[Client C] Connecté avec succès. Attente d'un message...\n");
    char recv_msg[128];
    int n = recv(sock, recv_msg, sizeof(recv_msg)-1, 0);
    if (n > 0) {
        recv_msg[n] = '\0';
        printf("[Client C] Message du serveur : %s\n", recv_msg);
        if (strncmp(recv_msg, "ID:", 3) == 0) {
            printf("[Client C] Numéro attribué : %s\n", recv_msg + 3);
        } else {
            printf("[Client C] Le serveur a refusé la connexion.\n");
            close(sock);
            return;
        }
    } else {
        perror("[Client C] Erreur à la réception du message");
        close(sock);
        return;
    }

    printf("[Client C] Envoi du pseudo : %s\n", username);
    if (send(sock, username, strlen(username), 0) < 0) {
        perror("[Client C] Erreur lors de l'envoi du pseudo");
    }

    printf("[Client C] Connexion terminée. Appuyez sur Entrée pour fermer.\n");
    getchar();
    close(sock);
}


void serveur() {
    printf("[Serveur] Lancement du serveur Python...\n");
    int result = system("../.venv/bin/python3 ../LAN/serveur.py");
    if (result != 0) {
        printf("[Serveur] Erreur lors du lancement du serveur Python.\n");
    }
}