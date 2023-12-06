/* damit inet_aton() deklariert wird */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <poll.h>

#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "sharedMemory.h"
#include "sysprak-client.h"
#include "performConnection.h"
#include "config.h"

#define BUF 1024

int flag = 0;

in_addr_t resolveAddress(char *domainName)
{
    // Erstellen der structs
    struct addrinfo *result, *res, hints;
    int error;

    // limit lookup to ipv4 and socket streams
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    /* resolve the domain name into a list of addresses */
    printf("resolve domain name \"%s\" ...\n", domainName);
    error = getaddrinfo(domainName, portnumberString, &hints, &result);
    if (error != 0)
    {
        // indicates failure in the contacted system
        if (error == EAI_SYSTEM)
        {
            perror("getaddrinfo");
        }
        else if (error == EAI_NONAME)
        {
            // Service not reachable
            fprintf(stderr, "error  %s\n", gai_strerror(error));
        }
        else
        {
            fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
        }
        freeaddrinfo(result);
        exit(EXIT_FAILURE); // TODO look into atexit()
    }

    char hostname[NI_MAXHOST];
    /* loop over returned results and do inverse lookup */
    for (res = result; res != NULL; res = res->ai_next)
    {
        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
        if (error != 0)
        {
            fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
            continue;
        }
        if (*hostname != '\0')
        {
            // exit loop on first found hostname
            printf("found host adress: %s\n", hostname);
            break;
        }
    }
    freeaddrinfo(result);
    return inet_addr(hostname); // returns inet address
}

// Funktion zum Abrufen, Prüfen und Ausgeben der vom Server versendeten Nachrichten
int checkRecv(int sockfd, char *buffer, int bufSize)
{
    ssize_t size;
    // Nachricht vom Server vollständig in Buffer einlesen
    int readCounter = 0;
    char c = ' ';
    do
    {
        size = recv(sockfd, &c, 1, 0);
        if (size >= 0)
        {
            buffer[readCounter++] = c;
            buffer[readCounter] = '\0';
        }
        else
        {
            printf("Recieving Message failed...\n");
        }

    } while (c != '\n' && bufSize > readCounter);

    // Nachricht auslesen
    if ((strncmp(buffer, "-", 1)) == 0)
    {
        // Negative Rückmeldung Server
        printf("S: %s \n", buffer);
        printf("Verbindung zum Server wird getrennt.\n");
    }
    else if ((strncmp(buffer, "+", 1)) == 0)
    {
        // Positive Rückmeldung vom Server
        printf("S: %s \n", buffer);
    }
    else
    {
        flag = 1; // Flag zum Beenden der infinite loop in performConnection()
    }

    return flag;
}

// Funktion fürs Pattern Matching der vom Server versendeten Daten
bool stringMesCompare(char *message, char *messageStart)
{
    if((strlen(message)>0 &&  message[0] == '\0' )|| (strlen(messageStart)>0 && messageStart[0] == '\0')) return false;
    if ((strlen(messageStart)>0 &&strncmp(message, messageStart, strlen(messageStart)) == 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Funktion zum Konkatenieren dreier Strings für die Client-Nachrichten
char *concatenate(const char *a, char *b, const char *c)
{
    int sizeString = strlen(a) + strlen(b) + strlen(c);
    char *str = (char *)malloc(sizeString);
    strcpy(str, a);
    strcat(str, b);
    strcat(str, c);

    return str;
}

// Funktion zum anstoßen des Thinkers
int initiateMove()
{
    printf("Sending Signal to Thinker..\n");
    kill(getppid(), SIGUSR1);
    return 0;
}

// Funktion zum Versenden der Client-Nachrichten
int checkSend(int sockfd, char *buffer)
{
    char message[BUF];
    memset(message, 0, BUF);
    strcpy(message, buffer);
    // bzero(buffer, BUF - 1);
    if (stringMesCompare(message, "+ MNM Gameserver") == true)
    {
        strcpy(buffer, "VERSION 3.0\n");
        printf("C: %s \n", buffer);
        send(sockfd, buffer, strlen(buffer), 0);
    }
    else if (stringMesCompare(message, "+ Client version accepted") == true)
    {
        strcpy(buffer, "ID ");
        strcat(buffer, shm_server_ptr->gameName);
        strcat(buffer, "\n");
        printf("C: %s \n", buffer);
        send(sockfd, buffer, strlen(buffer), 0);
    }
    else if (stringMesCompare(message, "+ PLAYING") == true)
    {
        char gameKind[10];
        sscanf(message, "+ %*s %s", gameKind);
        if (stringMesCompare(gameKind, confstruct.gamekind) == false)
        {
            printf("Spielart ist nicht %s. Client wird beendet.\n", confstruct.gamekind);
            return 0;
        }
        strcpy(shm_server_ptr->gameKind, gameKind);
    }
    else if (stringMesCompare(message, "+ ENDPLAYERS"))
    {
    }
    else if (stringMesCompare(message, "+ MOVEOK"))
    {
    }
    else if (stringMesCompare(message, "+ OKTHINK"))
    {
        shm_server_ptr->needNextMove = 1;
        initiateMove();
        // Code block for testing reading part of Pipe with valid turn

        // Pipe in richtiges format bringen
        fd_set set;
        FD_ZERO(&set);
        FD_SET(pipe_fd[0], &set);

        // Maximale Zeit einstellen die gewartet werden soll
        int time = (shm_server_ptr->maxTime - 300);
        struct timeval timeout;
        timeout.tv_sec = time / 1000;
        timeout.tv_usec = time % 1000 * 1000;

        // Listen to Pipe and wait max timout long
        int rv = select(pipe_fd[0] + 1, &set, NULL, NULL, &timeout);
        if (rv == -1)
        {
            perror("select");
        }
        else if (rv == 0)
        {
            printf("Pipe timeout!\n");
        }
        else
        {
            int len = 7;
            char validMove[7];
            read(pipe_fd[0], validMove, len);
            printf("Pipe Recieved: %s \n", buffer);

            strcpy(buffer, "PLAY ");
            strcat(buffer, validMove);
            strcat(buffer, "\n");

            printf("C: %s \n", buffer);
            send(sockfd, buffer, strlen(buffer), 0);
        }
    }
    else if (stringMesCompare(message, "+ MOVE"))
    { // S: + MOVE 〈〈 Maximale Zugzeit 〉〉
        int maxTime = 0;
        sscanf(message, "+ MOVE %d", &maxTime);
        printf("Max Move Time is: %d ms\n", maxTime);
        shm_server_ptr->maxTime = maxTime;
    }
    else if (stringMesCompare(message, "+ TOTAL"))
    { // S: + TOTAL 〈〈 Mitspieleranzahl 〉〉
        int maxPlayer = 0;
        sscanf(message, "+ TOTAL %d", &maxPlayer);
        printf("Insgesamte Spieler: %d\n", maxPlayer);
        shm_server_ptr->players = maxPlayer;
    }
    else if (stringMesCompare(message, "+ 0") || stringMesCompare(message, "+ 1"))
    { // S: + 〈〈 Mitspielernummer 〉〉 〈〈 Mitspielername 〉〉 〈〈 Bereit 〉〉
        int player = 0;
        int ready = 0;
        char name[20];
        sscanf(message, "+ %d %[^\n] %d", &player, name, &ready);
        name[strlen(name) - 2] = '\0';
        printf("Gegner %d namens %s ist %s\n", player, name, (ready == true ? "bereit" : "nicht bereit"));
        strcpy(shm_player_ptr->name, name);
        shm_player_ptr->player = player;
        shm_player_ptr->ready = ready;
        printPlayerInfo(shm_player_ptr);
    }
    else if (stringMesCompare(message, "+ CAPTURE"))
    { // S: + CAPTURE 〈〈 Anzahl zu schlagender Steine 〉〉
        int tilesToCapture = 0;
        sscanf(message, "+ CAPTURE %d", &tilesToCapture);
        printf("Tiles to Capture: %d \n", tilesToCapture);
        shm_server_ptr->tilesToCapture = tilesToCapture;
    }
    else if (stringMesCompare(message, "+ YOU"))
    { // S: + YOU 〈〈 Mitspielernummer 〉〉 〈〈 Mitspielername 〉〉
        int player = 0;
        char position[20];
        sscanf(message, "+ YOU %d %[^\n]", &player, position);
        printf("You are Player %d Named: %s\n", player, position);
        shm_server_ptr->player = player;
        // TODO store Player Name
    }
    else if (stringMesCompare(message, "+ PIECELIST"))
    { // S: + PIECELIST 〈〈 Anzahl Mitspieler 〉〉,〈〈 Anzahl Steine 〉〉
        int playerCount = 0;
        int tiles = 0;
        sscanf(message, "+ PIECELIST %d,%d", &playerCount, &tiles);
        printf("Number of Players: %d \n", playerCount);
        printf("Tiles: %d \n", tiles);
        shm_server_ptr->players = playerCount;
        shm_server_ptr->tilesLeft = tiles;
    }
    else if (stringMesCompare(message, "+ PIECE"))
    { // S: + PIECE〈〈 Mitspielernummer 〉〉.〈〈 Steinnummer 〉〉 〈〈 Position 〉〉
        int player = 0;
        int tileNumber = 0;
        char position[3];
        sscanf(message, "+ PIECE%d.%d %s", &player, &tileNumber, position);
        printf("Tile %d of Player %d is on Postion %s\n", tileNumber, player, position);
        strcpy(shm_server_ptr->tiles[player][tileNumber], position);
    }
    else if (stringMesCompare(message, "+ ENDPIECELIST"))
    {
        strcpy(buffer, "THINKING\n");
        printf("C: %s \n", buffer);
        send(sockfd, buffer, strlen(buffer), 0);

        //printServerInfo(shm_server_ptr);
        //send(sockfd, "THINKING\n", 10, 0);
    }
    else if (stringMesCompare(message, "+ WAIT"))
    {
        strcpy(buffer, "OKWAIT\n");
        printf("C: %s \n", buffer);
        send(sockfd, buffer, strlen(buffer), 0);
    }
    else if (stringMesCompare(message, "+ PLAYER0WON"))
    {
        char isWinner[4];
        sscanf(message, "+ PLAYER0WON %s", isWinner);
        if(stringMesCompare(isWinner,"Yes")){ 
        printf("Spieler 1 hat Gewonnen\n");}  
    }
    else if (stringMesCompare(message, "+ PLAYER1WON"))
    {
        char isWinner[4];
        sscanf(message, "+ PLAYER0WON %s", isWinner);
        if(stringMesCompare(isWinner,"Yes")){ 
        printf("Spieler 2 hat Gewonnen\n");} 
    }
    else if (stringMesCompare(message, "+ QUIT"))
    {
        printf("Spiel zu Ende Verbindung zum Server wird abgebaut...\n");

    }
    else if (stringMesCompare(message, "+ Already happy with your AI?"))
    {
    } 
    else if (stringMesCompare(message, "+ "))
    {
        char gameName[124];
        sscanf(message, "+ %s", gameName);
        printf("Spiel Lobby Namens: %s\n",gameName);
        strcpy(shm_player_ptr->name, gameName);

        if (player == 1 || player == 2)
        {
            char playerNr[12];
            sprintf(playerNr, "%d", player-1);
            strcpy(buffer, "PLAYER ");
            strcat(buffer, playerNr);
            strcat(buffer, "\n");
            printf("C: %.8s \n", buffer);
            send(sockfd, buffer, strlen(buffer), 0);
        }
        else
        {
            strcpy(buffer, "PLAYER\n");
            // printf("%s\n", buffer);
            printf("C: %s \n", buffer);
            send(sockfd, buffer, strlen(buffer), 0);
        }
    }
    else
    {
        printf("Unhandled send for message: %s", message);
    }

    return 0;
}

int performConnection(int sockfd)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = resolveAddress(confstruct.hostname);
    servaddr.sin_port = htons(confstruct.portnumber);
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
    {
        printf("connected to the server..\n");
    }

    // Communication
    char buffer[BUF];
    memset(buffer,0,BUF);

    // Per Schleife prüfen, ob noch Nachricht in Buffer von Server
    // Per If prüfen, welche Nachricht vom Server

    do
    {
        checkRecv(sockfd, buffer, BUF);
        if ((strncmp(buffer, "+", 1)) == 0)
            checkSend(sockfd, buffer);
    } while (flag != 1);

    return 0;
}
