#include <stdlib.h>
#include "sharedMemory.h"
#include "sysprak-client.h"
#include "performConnection.h"
    
// SHM Segment erstellen 
int createSHM(int structsize)
{
    key_t shm_key = IPC_PRIVATE;
    int shm_id = -1;
    if ((shm_id = shmget(shm_key, structsize, IPC_CREAT | 0666)) == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }else
        {
            return (shm_id);
        }
        
}

// SHM an Prozess anbinden
void* linkSHM(int shm_id)
{
    void* shm_ptr = NULL;
    if ((shm_ptr = shmat(shm_id, NULL, 0)) == NULL)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }else
        {
            return (shm_ptr);
        }
}

// Verbindung aufheben
void detachSHM(void* shm_ptr)
{
    if (shmdt(shm_ptr) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

// SHM Segment löschen
void deleteSHM(int shm_id)
{

    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

}

//Server Info printen
void printServerInfo(serverinfo* si)
{
    printf("-----Server Info-----\n");
    printf("Game Kind: %s\n", si->gameKind);
    printf("Game Name: %s\n", si->gameName);
    printf("Player Number: %d\n", si->player);
    printf("Player Count: %d\n", si->players);
    printf("Spielzug Zeit: %dms\n",shm_server_ptr->maxTime);
    printf("Stein des Feindes die geschmissen werden können: %d\n", si->tilesToCapture);
    printf("Steine pro Spieler: %d\n", si->tilesLeft);
    printf("Process IDs (Parent/Child): %d %d\n", si->pids[0], si->pids[1]);
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 9; j++) {
                //printf("Spieler [%d] Stein [%d] Position [%s]\n", i, j, (si->tiles[i][j]));
        }
    }
    printf("---------------------\n");
    
}
//Player Info printen
void printPlayerInfo(playerinfo* pi){
    printf("-----Player Info-----\n");
    printf("Player Number: %d\n", pi->player);
    printf("Name: %s\n", pi->name);
    printf("Ready: %d\n", pi->ready);
    printf("---------------------\n");
}

