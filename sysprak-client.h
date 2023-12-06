#pragma once 
#include "sharedMemory.h"


extern char gameId[14];
extern int player;
char portnumberString[12];

// Variablendeklaration
char gameId[14];
int player;
long gameIdSize;
int sockfd; 
int shmid_serverinfo;
int shmid_playerinfo; 
int pid;
int status;
serverinfo *shm_server_ptr;
playerinfo *shm_player_ptr;
int pipe_fd[2];


// Funktionsdeklarationen
void printHilfe();
void think();
void shm_cleanup(void);