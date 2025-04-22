#ifndef LAN_H
#define LAN_H

#include "crossplatform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TCP_PORT 12345

#define STATUT_FILE "../LAN/tmp/serveur_statut.txt"

#include "../JEU/jeu.h"

#define BUFFER_SIZE 1024

void client(char * username);
void serveur();

void attendre_serveur();

void get_data(socket_t sock, long int * received, char * server_response,int num,  int * quit);

#endif
