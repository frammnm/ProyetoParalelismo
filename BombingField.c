/* Archivo: BombingField.c
 * Contiene toda la funcionalidad para el ejercicio A del proyecto.
 * Autores: 
   - Francisco Martinez  09-10502
   - Gabriel Alvarez     09-10029
*/ 

#define TARGET_ARGUMENTS 3
#define BOMB_ARGUMENTS 4

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"

int N, T, B;
int *targets;
int *bombs;

void explosion(int N, int battleField[N][N], int x, int y, int power);
void bomb_battlefield(int N, int battleField[N][N], int pos, int numBombs, int me);
void distribute_work(int info[2], int numProcessors, int me);
int process_line(int array[], char *line, int pos);
int process_file(char *name);

/*
###############################################################################
#############################         main         ############################
###############################################################################
*/
int main(int argc, char *argv[]) {
  int i, j, root = 0;
  int me, numProcessors;
  int info[2];
  MPI_Status status;

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &me);
  MPI_Comm_size (MPI_COMM_WORLD, &numProcessors);

  if (process_file(argv[1]) == -1)
    return 0;

  int battleField[N][N];

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
      battleField[i][j] = 0;
  }

  for (i = 0; i < T; i++) {
    int x = targets[TARGET_ARGUMENTS * i];
    int y = targets[TARGET_ARGUMENTS * i + 1];
    battleField[x][y] = targets[TARGET_ARGUMENTS * i + 2];
  }

  if (me == root) {
    distribute_work(info, numProcessors, me);
  } else {
    MPI_Recv(info, 2, MPI_INT, root, 1, MPI_COMM_WORLD, &status);
  }

  bomb_battlefield(N, battleField, info[0], info[1], me);

  if (me == root) {
    int final_battleField[N*N];

    for (i = 1; i < numProcessors; i++) {
      MPI_Recv(final_battleField, N*N, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
      suma final_battleField battleField
    }

  }

  free(targets);
  free(bombs);

  MPI_Finalize();

  return 0;

}

/*
###############################################################################
#############################       explosion      ############################
###############################################################################
*/
void explosion(int N, int battleField[N][N], int x, int y, int power) {

  if ((0 <= x) && (x < N) && (0 <= y) && (y < N)) {
    if (battleField[x][y] > 0) {
      battleField[x][y] -= power;
      if (battleField[x][y] <= 0) battleField[x][y] = 0;
    } else if (battleField[x][y] < 0) {
      battleField[x][y] += power;
      if (battleField[x][y] >= 0) battleField[x][y] = 0;
    } else {
      battleField[x][y] = 0;
    }
  }
}

/*
###############################################################################
#############################   bomb_battleField   ############################
###############################################################################
*/
void bomb_battlefield(int N, int battleField[N][N], int pos, int numBombs, int me) {
  int i, j, x , y, radius, power;

  pos = pos / BOMB_ARGUMENTS;
  for (i = pos; i < pos + numBombs; i++) {
    x = bombs[i*BOMB_ARGUMENTS];
    y = bombs[i*BOMB_ARGUMENTS + 1];
    radius = bombs[i*BOMB_ARGUMENTS + 2]; 
    power = bombs[i*BOMB_ARGUMENTS + 3];

    explosion(N, battleField, x, y, power);
    explosion(N, battleField, x - radius, y, power);
    explosion(N, battleField, x, y - radius, power);
    explosion(N, battleField, x - radius, y - radius, power);
    explosion(N, battleField, x + radius, y, power);
    explosion(N, battleField, x, y + radius, power);
    explosion(N, battleField, x + radius, y + radius, power);
  }

  if (me != 0) MPI_Send(battleField, N*N, MPI_INT, 0, 1, MPI_COMM_WORLD);
}

/*
###############################################################################
#############################    distribute_work   ############################
###############################################################################
*/
void distribute_work(int info[2], int numProcessors, int me) {
    int numBombsPerProcessor[numProcessors];
    int pos = 0;
    int i = 0;

    // Send the number of bombs each processor will handle.
    if (B >= numProcessors) { // Number of bombs >= number of processors. 
      int quotient = B / numProcessors;
      int rm = B % numProcessors;
  
      for (i = 0; i < numProcessors; i++) {
        if (rm > 0) numBombsPerProcessor[i] = quotient + 1;
        else numBombsPerProcessor[i] = quotient;
        rm--;
        if (i > 0) {
          info[0] = pos;
          info[1] = numBombsPerProcessor[i];
          MPI_Send(info, 2, MPI_INT, i, 1, MPI_COMM_WORLD);
          pos = pos + numBombsPerProcessor[i]*BOMB_ARGUMENTS;
        }
      }
    } else { // Number of bombs < number of processors.
      int numBombs = B;

      for (i = 0; i < numProcessors; i++) {
        if (numBombs > 0) numBombsPerProcessor[i] = 1;
        else numBombsPerProcessor[i] = 0;
        numBombs--;
        if (i > 0) {
          info[0] = pos;
          info[1] = numBombsPerProcessor[i];
          MPI_Send(info, 2, MPI_INT, i, 1, MPI_COMM_WORLD);
          pos = pos + numBombsPerProcessor[i]*BOMB_ARGUMENTS;
        }
      }
    }
    info[0] = pos;
    info[1] = numBombsPerProcessor[0];
}

/*
###############################################################################
#############################     process_line     ############################
###############################################################################
*/
int process_line(int array[], char *line, int pos) {
  char *token;
  char *string;
  int i = pos;
  int n, j;

  token = strtok(line, " ");
  array[i] = atoi(token);
  i++;

  while (token != NULL) {
    token = strtok(NULL, " ");
    if (token == NULL) break;
    array[i] = atoi(token);
    i++;
  }

  return i;
}

/*
###############################################################################
#############################     process_file     ############################
###############################################################################
*/
int process_file(char *name) {
  FILE *fp;
  char line[256];
  int c, i, j, pos;

  if ((fp = fopen(name, "r")) == NULL) {
    printf("The input file \"%s\" doesn't exist!\n", name);
    return -1;
  };

  c = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (c == 0) N = atoi(line);
    else if (c == 1) {
      T = atoi(line);
      i = 0;
      pos = 0;
      targets = (int *) malloc(sizeof(int) * T * TARGET_ARGUMENTS);
      assert(targets != NULL);
    } else {
      if (i < T) {
        pos = process_line(targets, line, pos);
        i++;
      } else if (i == T) {
        j = 0;
        pos = 0;
        B = atoi(line);
        bombs = (int *) malloc(sizeof(int) * B * BOMB_ARGUMENTS);
        assert(bombs != NULL);
        i++;
      }
      else {
        pos = process_line(bombs, line, pos);
        j++;
      }
    }
    c++;
  };

  fclose(fp);
  return 0;
}
