#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"



//öffnet eine Config-Datei, Param: Dateiname filename
void openConfig(char *filename){ 
	char name[256];
	strcpy(name, filename);
	FILE *conffile;
	conffile = fopen(name,"r");
    if (conffile == NULL) { 
	// Datei lässt sich nicht öffnen - Default wird verwendet
    char defaultname[256] = "client.conf";
	strcpy(name, defaultname);
	printf("Default Config File wird verwendet %s\n", name);
		if((conffile=fopen(name,"r")) == NULL) { 
		perror("Fehler beim Öffnen der Default Config File"); 
		}
		else{
		printf("Default Config File geöffnet\n");
		readConfig(conffile);
		fclose(conffile);
		}
 	}
	else{
		printf("Übergebene Config File %s geöffnet\n", name);
		readConfig(conffile);
		fclose(conffile);
		}
}


/*
Geöffnete Config-Datei wird verarbeitet
und ausgeleseneDaten werden im Struct "configstruct" gespeichert.
Das Struct wurde in "config.h" erstellt  
*/

void readConfig(FILE *file){
	char *line = NULL;
	size_t len = 0;
	size_t nread;
	while((int)(nread = getline(&line,&len,file)) != -1){ //liest zeilenweise die Config-Datei

	int linelength = strlen(line);
	char befeq[linelength];
	char afteq[linelength];

	sscanf(line," %s = %s ", befeq, afteq);
	//sscanf nimmt alle vorherigen und folgenden Leerzeichen weg
		if(strcmp(befeq, "hostname") == 0){
			strcpy(confstruct.hostname, afteq);
		}
		if(strcmp(befeq, "portnumber") == 0){
			sscanf(afteq, "%d", &confstruct.portnumber);
			printf("%d", confstruct.portnumber);
			//confstruct.portnumber = atoi(afteq);
		}
		if(strcmp(befeq, "gamekind") == 0){
			strcpy(confstruct.gamekind, afteq);
		}		
	}
	printf("Config File wurde erfolgreich gelesen\n");

	free(line);


	/* Fehlerbehandlung falls ein Parameter = NULL ? */
}

