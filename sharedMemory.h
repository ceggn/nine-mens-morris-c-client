#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

// Struktur für gemeinsam genutzten Speicherbereich
typedef struct server_info
{
    char gameKind[8]; // Gamekind
    char gameName[124]; // Vom Gameserver übermittelter, individueller Spielname
    int player; // Unsere Spielernummer
    int players; // Anzahl der Spieler
    pid_t pids[2]; // Prozess-IDs (PID) der beiden Prozesse
    int maxTime; // MAximale Antwortzeit
    int tilesToCapture; // Steine des Feindes
    int tilesLeft; // Steine unseren spielers 
    char tiles[2][9][3]; // Alle Tiles der Spieler [Spieler][tile nummer]  
    int needNextMove;
} serverinfo;

// Struktur für Spieler-Eigenschaften
typedef struct player_info
{
    int player; // Spielernummer
    char name[17]; // Spielername
    int ready; // Flag, ob Spieler bereits registriert / bereit ist oder nicht
} playerinfo;


// Funktionen deklarieren
int createSHM(int structsize);
void* linkSHM(int shm_id);
void detachSHM(void* shm_ptr);
void deleteSHM(int shm_id);
void printServerInfo(serverinfo* si);
void printPlayerInfo(playerinfo* pi);
