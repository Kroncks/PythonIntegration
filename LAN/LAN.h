#ifndef LAN_H
#define LAN_H

#include "crossplatform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TCP_PORT 12345

#define STATUT_FILE "../LAN/tmp/serveur_statut.txt"

void client(char * username);
void serveur();


#endif
