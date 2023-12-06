#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sharedMemory.h"
#include "sysprak-client.h"

// Anlegen der KI variablen
char fieldNames[3][8][3] = {{"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"},
                            {"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"},
                            {"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7"}};
int gameField[3][8] = {{0}}; // Spielfeld mit 1 für Spieler 1 etc.
int weight[3][8] = {{1}};    // die gewichtung wird hochgesezt durch einzelen KI berechnungen und das feld mit der höchsten gewichtung wird als zug genommen
int tilesNotPlaced = 0;
char validMoves[144][7] = {};
int validCount = 0;
int freeFields = 24;
char moveToBeSend[7] = "";

void resetInitialVariables()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      gameField[i][j] = 0;
      weight[i][j] = 1;
    }
  }
  tilesNotPlaced = 0;
  for (int i = 0; i < validCount; i++)
  {
    validMoves[i][0] = '\0';
  }
  validCount = 0;
  freeFields = 24;
  memset(moveToBeSend, 0, strlen(moveToBeSend));
}

char getI2C(int a)
{
  return (a == 1 ? 'O' : a == 2 ? 'X'
                                : '+');
}
void printWeights()
{
  printf("____Weightfeld____\n");
  printf("%d------%d------%d\n", (weight[0][0]), (weight[0][1]), (weight[0][2]));
  printf("| %d----%d----%d |\n", (weight[1][0]), (weight[1][1]), (weight[1][2]));
  printf("| | %d--%d--%d | |\n", (weight[2][0]), (weight[2][1]), (weight[2][2]));
  printf("%d-%d-%d     %d-%d-%d\n", (weight[0][7]), (weight[1][7]), (weight[2][7]), (weight[2][3]), (weight[1][3]), (weight[0][3]));
  printf("| | %d--%d--%d | |\n", (weight[2][6]), (weight[2][5]), (weight[2][4]));
  printf("| %d----%d----%d |\n", (weight[1][6]), (weight[1][5]), (weight[1][4]));
  printf("%d------%d------%d\n", (weight[0][6]), (weight[0][5]), (weight[0][4]));
}

void printRawGameField()
{
  printf("____Gamefield____\n");
  printf("%d------%d------%d\n", (gameField[0][0]), (gameField[0][1]), (gameField[0][2]));
  printf("| %d----%d----%d |\n", (gameField[1][0]), (gameField[1][1]), (gameField[1][2]));
  printf("| | %d--%d--%d | |\n", (gameField[2][0]), (gameField[2][1]), (gameField[2][2]));
  printf("%d-%d-%d     %d-%d-%d\n", (gameField[0][7]), (gameField[1][7]), (gameField[2][7]), (gameField[2][3]), (gameField[1][3]), (gameField[0][3]));
  printf("| | %d--%d--%d | |\n", (gameField[2][6]), (gameField[2][5]), (gameField[2][4]));
  printf("| %d----%d----%d |\n", (gameField[1][6]), (gameField[1][5]), (gameField[1][4]));
  printf("%d------%d------%d\n", (gameField[0][6]), (gameField[0][5]), (gameField[0][4]));
}

void printGameField()
{
  printf("____Spielfeld____\n");
  printf("%c------%c------%c\n", getI2C(gameField[0][0]), getI2C(gameField[0][1]), getI2C(gameField[0][2]));
  printf("| %c----%c----%c |\n", getI2C(gameField[1][0]), getI2C(gameField[1][1]), getI2C(gameField[1][2]));
  printf("| | %c--%c--%c | |\n", getI2C(gameField[2][0]), getI2C(gameField[2][1]), getI2C(gameField[2][2]));
  printf("%c-%c-%c     %c-%c-%c\n", getI2C(gameField[0][7]), getI2C(gameField[1][7]), getI2C(gameField[2][7]), getI2C(gameField[2][3]), getI2C(gameField[1][3]), getI2C(gameField[0][3]));
  printf("| | %c--%c--%c | |\n", getI2C(gameField[2][6]), getI2C(gameField[2][5]), getI2C(gameField[2][4]));
  printf("| %c----%c----%c |\n", getI2C(gameField[1][6]), getI2C(gameField[1][5]), getI2C(gameField[1][4]));
  printf("%c------%c------%c\n", getI2C(gameField[0][6]), getI2C(gameField[0][5]), getI2C(gameField[0][4]));
  printf("O für P1 und X für P2\n ");
};

void think()
{
  printf("Spielzug wird berechnet...\n");
  if (shm_server_ptr->needNextMove == 0)
  {
    return; // Not intended call of think method
  }
  shm_server_ptr->needNextMove = 0; // Flag zurücksetzen das ein nächster zug benötigt wird
  resetInitialVariables();

  // A aleine bedeutete der Stein wurde noch nciht gesezt und C bedeutet er wurde geschlagen
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 9; j++)
    {
      int row;
      char position[3];
      strcpy(position, (shm_server_ptr->tiles[i][j]));
      if (strcmp(position, "A") == 0)
      {
        // do something tile not placed
        if (shm_server_ptr->player == i)
        {
          tilesNotPlaced++;
        }
      }
      else if (strcmp(position, "C") == 0)
      {
        // do something tile already destroyed
      }
      else
      {
        row = (strstr(position, "A") != NULL) ? 0 : (strstr(position, "B") != NULL) ? 1
                                                : (strstr(position, "C") != NULL)   ? 2
                                                                                    : -1;
        int pos = (int)position[1] - 48;      // char to int, ASCI code starts at 48
        gameField[row][pos] = i == 0 ? 1 : 2; // plazieren der tiles in dem Spielfeld Array
        weight[row][pos] = 0;
        freeFields--;
        //printf("Spieler [%d] Stein [%d] Position [%s] auf Reihe [%d] und Positionsnummer [%d] \n", i, j, (shm_server_ptr->tiles[i][j]), row, pos);
        // do something
      }
    }
  }
  //printGameField();
  //printRawGameField();

  // Phase 2 Moving Tiles by 1
  if (tilesNotPlaced == 0 && shm_server_ptr->tilesToCapture == 0)
  {
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        int field = gameField[i][j];
        weight[i][j] = 0;
        char moveCombination[7] = "..:..";
        strncpy(moveCombination, fieldNames[i][j], 2);
        // printf("Feld %s Player %d\n ",fieldNames[i][j],field);
        if (field == (shm_server_ptr->player + 1))
        { // field empty and able to move to
          //printf("Feld %s Player %d and move %s\n ", fieldNames[i][j], field, moveCombination);

          if (gameField[i][(j + 7) % 8] == 0)
          {
            strncpy(moveCombination + 3, fieldNames[i][(j + 7) % 8], 2);
            strncpy(validMoves[validCount], moveCombination, 6);
            validCount++;
            //printf("Left %s  %d %d %s\n ", moveCombination,i, (j + 7) % 8,fieldNames[i][(j + 7) % 8]);
          }
          if (gameField[i][(j + 1) % 8] == 0)
          {
            strncpy(moveCombination + 3, fieldNames[i][(j + 1) % 8], 2);
            strncpy(validMoves[validCount], moveCombination, 6);
            validCount++;
            //printf("Right %s\n ", moveCombination);
          }
          if (j % 2 == 1)
          {
            if (i != 0 && gameField[i - 1][j] == 0)
            {
              strncpy(moveCombination + 3, fieldNames[i - 1][j], 2);
              strncpy(validMoves[validCount], moveCombination, 6);
              validCount++;
              //printf("Outer %s\n ", moveCombination);
            }
            if (i != 2 && gameField[i + 1][j] == 0)
            {
              strncpy(moveCombination + 3, fieldNames[i + 1][j], 2);
              strncpy(validMoves[validCount], moveCombination, 6);
              validCount++;
              //printf("Inner %s\n ", moveCombination);
            }
          }
        }
      }
    }
    int randomPhase2Move = rand() % validCount;
    strncpy(moveToBeSend, validMoves[randomPhase2Move], 7);
  }

  //printWeights();
  //printf("Tiles to capture: %i\n", shm_server_ptr->tilesToCapture);

  //Phase 3: Capture random enemy tile
  if (shm_server_ptr->tilesToCapture > 0)
  {
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        int field = gameField[i][j];
        if (field != 0 && field != (shm_server_ptr->player + 1))
        {
          strncpy(validMoves[validCount], fieldNames[i][j], 2);
          validCount++;
          // printf("Feld %s Player %d\n ",fieldNames[i][j],field);
        }
      }
    }

    int randomCapture = rand() % validCount;
    strncpy(moveToBeSend, validMoves[randomCapture], 2);
  }

  if (tilesNotPlaced > 0 && shm_server_ptr->tilesToCapture == 0){
    printf("Phase 1 Steine Plazieren");
    // Am besten geichteten zu auswählen
    int bestMoveRow = 0;
    int bestMoveNum = 0;
    int maxWeight = 0;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        int newWeight = weight[i][j];
        if (newWeight == maxWeight)
        { // bei gleichen gewicht ist es zufällig welcher zug genommen wird
          if (rand() % 2 == 0)
          {
            bestMoveRow = i;
            bestMoveNum = j;
            //printf("New Random Move %s %d|%d  w:%d old w:%d\n", fieldNames[bestMoveRow][bestMoveNum], bestMoveRow, bestMoveNum, newWeight, maxWeight);
            maxWeight = newWeight;
          }
        }
        else if (newWeight > maxWeight)
        { // der zug mit dem besseren gewicht wird übernommen
          bestMoveRow = i;
          bestMoveNum = j;
          maxWeight = newWeight;
          //printf("New Random Move %s %d|%d  w:%d old w:%d\n", fieldNames[bestMoveRow][bestMoveNum], bestMoveRow, bestMoveNum, newWeight, maxWeight);
        }
      }
    }
    strncpy(moveToBeSend, fieldNames[bestMoveRow][bestMoveNum], 3);
  }

  // Gültigen Spielzug schicken
  printf("Send into Pipe: %s\n", moveToBeSend);
  if ((write(pipe_fd[1], moveToBeSend, 7)) != 7)
  {
    perror(" Fehler bei write (). \n");
    exit(EXIT_FAILURE);
  }
}