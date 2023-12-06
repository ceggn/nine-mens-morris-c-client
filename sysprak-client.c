#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>

#include "sysprak-client.h"
#include "performConnection.h"
#include "config.h"
#include "sharedMemory.h"
#include "think.h"

// Hilfestellung zum Aufruf des Clients 
void printHilfe()
{
  printf("Hinweise zur Benutzung des Programms\n");
  printf("Moegliche Optionen:\n");
  printf("  -g <GAME-ID>: 13-stellige Game-ID (obligatorisch)\n");
  printf("  -p <{1,2}>: gewünschte Spielernummer (obligatorisch)\n");
  printf("  -c <CONFIG-FILE>: gewünschte Konfigurations-Datei (optional)\n");
}

int main(int argc, char **argv)
{
  // Variable zum Speichern des Config-File-Names 
  char fileName[256] = "\0";

  // Einlesen der Kommandozeilenparameter 
  int ret;
  while ((ret = getopt(argc, argv, "g:p:c")) != -1)
  {
    switch (ret)
    {
    case 'g': // Game-ID
      strncpy(gameId, optarg, 14);
      break;
    case 'p': // Spielernummer
      player = atoi(optarg);
      break;
    case 'c': // Config-File
      strncpy(fileName, optarg, 256);
      break;
    default:
      printHilfe();
      return 0;
      break;
    }
  }


  // Prüfen, ob eine Config-File mit übergeben wurde 
  if (fileName[0] == '\0')
  {
    strcpy(fileName, "client.conf");
    printf("Keine Konfigdatei angegeben. Es wird die Standard-Konfig verwendet.\n");
  }
  
  // Auslesen der Config-File 
  openConfig(fileName);
  //printf("Port %i\n", confstruct.portnumber);
  //printf("Host %s\n", confstruct.hostname);
  //printf("Spiel %s\n", confstruct.gamekind);

  //Konvertieren des Integers configStruct.portnumber in einen String zur weiteren Verarbeitung  
  if(sprintf(portnumberString, "%d", confstruct.portnumber) == -1)
  {
    perror("Fehler beim Konvertieren der Portnummer in einen String.");
    return EXIT_FAILURE;
  }

  //printf("Portnumber: %s\n", portnumberString);

  // Prüfen auf zulässige Eingaben für Game-ID und Spielernummer
  long gameIdSize = strlen(gameId);
  if (gameIdSize != 13)
  {
    printf("Fehler: Unzulässige Eingabe.\n  Die Game-ID muss genau 13 stellen haben.\n");
    printf("  Game-ID: %s ist aber nur %ld Zeichen lang.\n", gameId, gameIdSize);
    printf("Hier nochmal der Hinweis:\n");
    printHilfe();
    return -1;
  }
  else if (player != 1 && player != 2)
  {
    printf("Fehler: Unzulässige Eingabe.\n  Die Spielernummer muss 1 oder 2 sein.\n");
    printf("Hier nochmal der Hinweis:\n");
    printHilfe();
    return -1;
  }

  // Registieren des atexit()-Handler
  atexit(shm_cleanup);

  // Semaphore erstellen
  // sem_init(&semaAccess,0,1);

  // Erstellen des Shared Memory 
  shmid_serverinfo = createSHM(sizeof(struct server_info));
  shmid_playerinfo = createSHM(sizeof(struct player_info));

  // Erstellen und Verifizieren des Sockets 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    printf("Erstellen des Sockets fehlgeschlagen...\n");
    exit(0);
  }
  else
    printf("Socket wurde erfolgreich erstellt...\n");

  // Erstellen der Pipe zwischen Connector und Thinker
  // int pipe_fd[2];
  if (pipe(pipe_fd) < 0)
  {
    perror(" Fehler beim Einrichten der Pipe .");
    exit(EXIT_FAILURE);
  }

  // Erstellen eines Kindprozess
  int pid;
  if ((pid = fork()) < 0)
  {
    perror("Fehler beim Aufsplitten des Prozesses");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    // Kindprozess aka Connector
    // close(pipe_fd[1]); // Pipe Schreibseite schließen

    // SHM Segment linken
    shm_server_ptr = (serverinfo *)linkSHM(shmid_serverinfo);
    shm_player_ptr = (playerinfo *)linkSHM(shmid_playerinfo);
    shm_server_ptr->pids[0] = getppid();
    shm_server_ptr->pids[0] = getpid();
    shm_server_ptr->players = 2; // Anzahl der Spieler
    shm_server_ptr->player = player - 1;
    shm_server_ptr->needNextMove = false;
    strcpy(shm_server_ptr->gameName, gameId);
    printServerInfo(shm_server_ptr);
    // printf("P-Id Parent: %i, P-Id Child: %i, Spieleranzahl: %i\n", shm_server_ptr->pids[0],shm_server_ptr->pids[1], shm_server_ptr->players);

    // Pipe Leseseite auslesen
    /*char string [9];
    int n = read ( pipe_fd [0] , string , 8 );
    printf("Pipe Message of length %d: %s\n",n,string);*/

    performConnection(sockfd);
    close(sockfd);
    printf("Socket geschlossen...\n");
  }
  else
  {
    // Elternprozess
    // close(pipe_fd[0]); // Pipe Leseseite schließen

    //setup signal handle and link think method
    
    struct sigaction sa = {0};
    sa.sa_flags = 0;
    sa.sa_handler = &think;
    sigaction(SIGUSR1, &sa, NULL);
    
    //signal(SIGUSR1, &think);
    

    // SHM Segment linken
    shm_server_ptr = (serverinfo *)linkSHM(shmid_serverinfo);
    shm_player_ptr = (playerinfo *)linkSHM(shmid_playerinfo);
    printf("Number of players: %d\n", shm_player_ptr->player);

    // Warte auf das Beenden des Kindprozesses (verhindern Verwaisung)
    int status;
    while ((waitpid(pid, &status, WNOHANG)) == 0) // non blocking wait for child(Connector)
    {
      // solange kindprocess noch lebt
      // printf("Alive and Waiting status: %d\n",status);
      // sleep(1);
    }
    printf("My Child Process is no more.. %d\n", status);
    // Warte auf das Beenden des Kindprozesses (verhindern Verwaisung)
    wait(&status);
  }

  // Destroy Semaphore
  // sem_destroy(&semaAccess);

  return EXIT_SUCCESS;
}

// At-Exit Handler, der den geteilten Speicherbereich freigibt
void shm_cleanup(void)
{
  // Löschen der SHM Segmente
  deleteSHM(shmid_serverinfo);
  deleteSHM(shmid_playerinfo);
}
