#pragma once

#define BUFFERSIZE 1024

typedef struct{
    char hostname[BUFFERSIZE];
    int portnumber;
    char gamekind[BUFFERSIZE];
}config;

config confstruct;

void openConfig(char *filename);
void readConfig(FILE *file);