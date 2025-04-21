#include "LAN.h"

void client(char *username) {
    char buffer[128];

    printf("[Client C] Lancement de la détection du serveur via client.py...\n");
#ifdef _WIN32
    FILE *fp = popen("python.exe ../LAN/client.py", "r");
#else
    FILE *fp = popen("../.venv/bin/python3 ../LAN/client.py", "r");
#endif
    if (!fp) {
        perror("[Client C] Erreur appel script Python");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        printf("[Client C] Échec de la détection du serveur.\n");
        pclose(fp);
        return;
    }
    pclose(fp);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("[Client C] Serveur détecté à l'adresse : %s\n", buffer);

    if (network_init() != 0) {
        printf("[Client C] Erreur d'initialisation réseau.\n");
        return;
    }

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[Client C] Erreur création socket");
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
#ifdef _WIN32
    serv_addr.sin_addr.s_addr = inet_addr(buffer);
#else
    if (inet_pton(AF_INET, buffer, &serv_addr.sin_addr) <= 0) {
        perror("[Client C] Adresse invalide");
        socket_close(sock);
        return;
    }
#endif

    printf("[Client C] Tentative de connexion...\n");
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[Client C] Connexion échouée");
        socket_close(sock);
        network_cleanup();
        return;
    }

    printf("[Client C] Connecté. Envoi du pseudo : %s\n", username);
    send(sock, username, strlen(username), 0);

    printf("[Client C] Terminé. Appuyez sur Entrée.\n");
    getchar();
    socket_close(sock);
    network_cleanup();
}

void serveur() {
    printf("[Serveur] Lancement du serveur Python...\n");
#ifdef _WIN32
    int result = system("python.exe ../LAN/serveur.py");
#else
    int result = system("../.venv/bin/python3 ../LAN/serveur.py");
#endif
    if (result != 0) {
        printf("[Serveur] Erreur lors du lancement du serveur Python.\n");
    }
}