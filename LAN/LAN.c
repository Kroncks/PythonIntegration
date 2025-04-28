#include "LAN.h"

// Lit depuis le socket jusqu'à trouver un '\n'
size_t read_msg(socket_t sock, char *buffer, size_t size) {
    char ch;
    size_t i = 0;
    while (i < size - 1) {
        long int n = recv(sock, &ch, 1, 0);
        if (n <= 0) break;
        buffer[i++] = ch;
        if (ch == '\n') break;
    }
    buffer[i] = '\0';
    return i;
}

void wait_ready(socket_t sock) {
    printf("[Client C] Waiting for READY\n");
    char buffer[BUFFER_SIZE];
    while (1) {
        size_t n = read_msg(sock, buffer, BUFFER_SIZE);
        if (n <= 0) {
            fprintf(stderr, "[Client C] Déconnexion du serveur (READY)\n");
            return;
        }
        if (strstr(buffer, "READY")) break;
    }
    printf("[Client C] Serveur ready to launch game !\n[Launch]>");
    fflush(stdin);
    do {} while (getchar()!='a');  // Attente de l'utilisateur pour START?
}

void wait_start(socket_t sock) {
    printf("[Client C] Waiting for START e_e\n");
    char buffer[BUFFER_SIZE];
    while (1) {
        size_t n = read_msg(sock, buffer, BUFFER_SIZE);
        if (n <= 0) {
            fprintf(stderr, "[Client C] Déconnexion du serveur (START)\n");
            return;
        }
        if (strstr(buffer, "START")) break;
    }
    printf("[Client C] Game Start !!! \n");
}

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

    printf("[Client C] Connecté. En attente de l'attribution...\n");
    char server_response[32] = {0}; // petit buffer pour les petites communications
    long int received = recv(sock, server_response, sizeof(server_response) - 1, 0);
    if (received <= 0) {
        printf("[Client C] Erreur lors de la réception du message du serveur.\n");
        socket_close(sock);
        network_cleanup();
        return;
    }

    server_response[received] = '\0';

    if (strcmp(server_response, "FULL\n") == 0) {
        printf("[Client C] La partie est déjà complète. Déconnexion.\n");
        socket_close(sock);
        network_cleanup();
        return;
    }

    printf("[Client C] Identifiant reçu : %s\n", server_response);
    printf("[Client C] Envoi du pseudo : %s\n", username);
    send(sock, username, strlen(username), 0);


    Game game;
    if (server_response[3]-48 == 0) {
        wait_ready(sock);
        send(sock, "START?", 6, 0);
    }
    wait_start(sock);
    init_game(sock, &game, server_response[3]-48, username);
    jouer(sock, &game, server_response[3]-48);

    socket_close(sock);
    network_cleanup();
}


void serveur() { // lance le serveur en arrière plan o-o
    remove(STATUT_FILE);
    printf("[Serveur] Lancement du serveur Python...\n");
#ifdef _WIN32
    int result = system("start /B python.exe ../LAN/serveur.py");
#else
    int result = system("../.venv/bin/python3 ../LAN/serveur.py &");
#endif
    if (result != 0) {
        printf("[Serveur] Erreur lors du lancement du serveur Python.\n");
    }
}

void attendre_serveur() {
    FILE *file;
    do {
        file = fopen(STATUT_FILE, "r");
    } while (!file);
    fclose(file);
}


void get_data(socket_t sock, long int * received, char * buffer,int num,  int * quit) {
    char server_response[BUFFER_SIZE];
    *received = recv(sock, server_response, BUFFER_SIZE - 1, 0);
    if (*received <= 0) {
        printf("[Client C] Déconnexion du serveur.\n");
        *quit = 1;
        return;
    }

    server_response[*received] = '\0';
    printf("[Client C] Message du serveur : %s\n", server_response);

    if (strcmp(server_response, "STOP") == 0) {
        printf("[Client C] Partie terminée. Fermeture du client.\n");
        *quit = 1;
        return;
    }

    if (server_response[0]-48 != num) {
        printf("[Client C] Donnée inatendue : (%d , %d)\n",server_response[0]-48 , num);
    } else {
        printf("[Client C] Donnée ok\n");
    }

    strcpy(buffer, server_response+3);
}