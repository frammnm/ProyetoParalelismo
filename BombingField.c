/* Archivo: battleField.c
 * Contiene toda la funcionalidad para el ejercicio A del proyecto.
 * Autores: 
   - Francisco Martinez  09-10502
   - Gabriel Alvarez     09-10029
*/ 

#define TARGET_ARGUMENTS 3
#define BOMB_ARGUMENTS 4

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"

int N, T, B;
int *targets;
int *bombs;

void battleField_explosion(int *results, int *battleField, int *bomb_power_matrix, 
                           int work, int displ, int me);
void set_power(int *bomb_power_matrix, int x, int y, int power);
void get_bomb_power_matrix(int *bomb_power_matrix, int pos, int numBombs, int me);
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
  int *bomb_power_matrix;
  int *battleField;
  MPI_Status status;

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &me);
  MPI_Comm_size (MPI_COMM_WORLD, &numProcessors);

  if (process_file(argv[1]) == -1) {
    MPI_Finalize();
    return 0;
  }

  if (N < numProcessors) {
    printf("The size N of the battle field is less than the number of processors!\n");
    MPI_Finalize();
    return 0;
  }

  battleField = (int *) malloc(sizeof(int *) * N * N);
  assert(battleField != NULL);
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
      battleField[i*N + j] = 0;
  }
  for (i = 0; i < T; i++) {
    int x = targets[TARGET_ARGUMENTS * i];
    int y = targets[TARGET_ARGUMENTS * i + 1];
    if ((0 <= x) && (x < N) && (0 <= y) && (y < N))
      battleField[x*N + y] = targets[TARGET_ARGUMENTS * i + 2];
  }

  if (me == root) {
    distribute_work(info, numProcessors, me);
  } else {
    MPI_Recv(info, 2, MPI_INT, root, 1, MPI_COMM_WORLD, &status);
  }

  bomb_power_matrix = (int *) malloc(sizeof(int) * N * N);
  assert(bomb_power_matrix != NULL);
  memset(bomb_power_matrix, 0, sizeof(int) * N * N);

  get_bomb_power_matrix(bomb_power_matrix, info[0], info[1], me);

  int sendcounts[numProcessors];
  int quotient = (N * N) / numProcessors;
  int rm = (N * N) % numProcessors;
  int displs[numProcessors];
  int sum = 0;

  for (i = 0; i < numProcessors; i++) {
    if (rm > 0) sendcounts[i] = quotient + 1;
    else sendcounts[i] = quotient;
    rm--;
    displs[i] = sum;
    sum += sendcounts[i];
  }

  int bomb_power_matrices[numProcessors][N*N];

  if (me == root) {
    for (i = 1; i < numProcessors; i++) {
      MPI_Recv(bomb_power_matrices[i], N*N, MPI_INT, i, 1, MPI_COMM_WORLD, 
               &status);
    }
    for (i = 0; i < N * N; i++) {
      bomb_power_matrices[0][i] = bomb_power_matrix[i];
    }
  }

  int recv_array[numProcessors][sendcounts[me]];

  for (i = 0; i < numProcessors; i++)
    MPI_Scatterv(&bomb_power_matrices[i], sendcounts, displs, MPI_INT, 
                 recv_array[i], sendcounts[me], MPI_INT, root, 
                 MPI_COMM_WORLD);

  for (i = 0; i < sendcounts[me]; i++) {
    sum = 0;
    for (j = 0; j < numProcessors; j++) 
      sum += recv_array[j][i];
    recv_array[0][i] = sum;
  }

  MPI_Allgatherv(&recv_array[0], sendcounts[me], MPI_INT, bomb_power_matrix, 
                 sendcounts, displs, MPI_INT, MPI_COMM_WORLD);

  int *results;

  results = (int *) malloc(sizeof(int) * 6);

  battleField_explosion(results, battleField, bomb_power_matrix, sendcounts[me], 
                        displs[me], me);

  if (me == root) {
    int recv_results[6];
    for (i = 1; i < numProcessors; i++) {
      MPI_Recv(recv_results, 6, MPI_INT, i, 1, MPI_COMM_WORLD, 
               &status);
      for (j = 0; j < 6; j++)
        results[j] += recv_results[j];
    }
    printf("Military Targets totally destroyed: %d\n", results[0]);
    printf("Military Targets partially destroyed: %d\n", results[1]);
    printf("Military Targets not affected: %d\n", results[2]);
    printf("Civilian Targets totally destroyed: %d\n", results[3]);
    printf("Civilian Targets partially destroyed: %d\n", results[4]);
    printf("Civilian Targets not affected: %d\n", results[5]);
  }

  free(results);
  free(battleField);
  free(bomb_power_matrix);
  free(targets);
  free(bombs);

  MPI_Finalize();

  return 0;

}

/*
###############################################################################
############################  battleField_explosion  ##########################
###############################################################################
*/
void battleField_explosion(int *results, int *battleField, int *bomb_power_matrix, 
                           int work, int displ, int me) {
  int i, j;
  int MT_totallyDestroyed = 0, MT_partiallyDestroyed = 0, MT_notTouched = 0;
  int CT_totallyDestroyed = 0, CT_partiallyDestroyed = 0, CT_notTouched = 0;

  for (i = displ; i < displ + work; i++) {
    if (battleField[i] < 0) {
      if (bomb_power_matrix[i] > -1*(battleField[i])) MT_totallyDestroyed += 1;
      else if (bomb_power_matrix[i] < -1*(battleField[i])) MT_partiallyDestroyed += 1;
      else if (bomb_power_matrix[i] == 0) MT_notTouched += 1;
    } else if (battleField[i] > 0) {
      if (bomb_power_matrix[i] > battleField[i]) CT_totallyDestroyed += 1;
      else if (bomb_power_matrix[i] < battleField[i]) CT_partiallyDestroyed += 1;
      else if (bomb_power_matrix[i] == 0) CT_notTouched += 1;
    }
  }
  results[0] = MT_totallyDestroyed;
  results[1] = MT_partiallyDestroyed;
  results[2] = MT_notTouched;
  results[3] = CT_totallyDestroyed;
  results[4] = CT_partiallyDestroyed;
  results[5] = CT_notTouched;

  if (me != 0) MPI_Send(results, 6, MPI_INT, 0, 1, MPI_COMM_WORLD);
}

/*
###############################################################################
#############################       set_power      ############################
###############################################################################
*/
void set_power(int *bomb_power_matrix, int x, int y, int power) {
  if ((0 <= x) && (x < N) && (0 <= y) && (y < N)) {
    bomb_power_matrix[N*x + y] += power;
  }
}

/*
###############################################################################
############################ get_bomb_power_matrix ############################
###############################################################################
*/
void get_bomb_power_matrix(int *bomb_power_matrix, int pos, int numBombs, 
                           int me) {
  int i, j, k, x , y, radius, power, r1, r2;

  pos = pos / BOMB_ARGUMENTS;
  for (i = pos; i < pos + numBombs; i++) {
    x = bombs[i*BOMB_ARGUMENTS];
    y = bombs[i*BOMB_ARGUMENTS + 1];
    radius = bombs[i*BOMB_ARGUMENTS + 2]; 
    power = bombs[i*BOMB_ARGUMENTS + 3];

    r1 = -radius;
    for (j = 0; j < radius*2 + 1; j++) {
      r2 = -radius;
      for (k = 0; k < radius*2 + 1; k++) {
        set_power(bomb_power_matrix, x + r1, y + r2, power);
        r2++;
      }
      r1++;
    }
  }

  if (me != 0) MPI_Send(bomb_power_matrix, N*N, MPI_INT, 0, 1, MPI_COMM_WORLD);
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
