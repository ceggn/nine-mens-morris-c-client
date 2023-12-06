#pragma once
#include <netdb.h>
#include <stdbool.h>

in_addr_t resolveAdress(char* domainName);
int checkRecv(int sockfd, char *buffer, int bufSize);
bool stringMesCompare(char *message, char *messageStart);
char *concatenateStrings(char *string1, char *string2, char *string3); 
int checkSend(int sockfd, char *buffer);
int performConnection(int sockfd);
