#ifndef LAN_H
#define LAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TCP_PORT 12345

void client(char * username);
void serveur();


#endif
