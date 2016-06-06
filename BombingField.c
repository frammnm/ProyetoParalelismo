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
//#include "mpi.h"

int N, T, B;
int **targets;
int **bombs;

/*
###############################################################################
#############################     process_line     ############################
###############################################################################
*/
void process_line(int *array, char *line) {
  char *token;
  int i = 0;

  token = strtok(line, " ");
  array[i] = atoi(token);
  i++;

  while (token != NULL) {
    token = strtok(NULL, " ");
    array[i] = atoi(token);
    i++;
  }
}

/*
###############################################################################
#############################     process_file     ############################
###############################################################################
*/
int process_file(char *name) {
  FILE *fp;
  char line[256];
  int c, i, j;

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
      targets = (int **) malloc(sizeof(int *) * T);
    } else {
      if (i < T) {
        targets[i] = (int *) malloc(sizeof(int) * TARGET_ARGUMENTS);
        process_line(targets[i], line);
        i++;
      } else if (i == T) {
        j = 0;
        B = atoi(line);
        bombs = (int **) malloc(sizeof(int *) * B);
        i++;
      }
      else {
        bombs[j] = (int *) malloc(sizeof(int) * BOMB_ARGUMENTS);
        process_line(bombs[j], line);
        j++;
      }
    }
    c++;
  };

  fclose(fp);

}

/*
###############################################################################
#############################         main         ############################
###############################################################################
*/
int main(int argc, char *argv[]) {
  int i, j;

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  if (process_file(argv[1]) == -1)
    return 0;

  printf("N = %d\n", N);
  printf("T = %d\n", T);
  printf("B = %d\n", B);

  for (i = 0; i < T; i++)
    for (j = 0; j < TARGET_ARGUMENTS; j++)
      printf("targets[%d][%d] = %d\n", i, j, targets[i][j]);

  for (i = 0; i < B; i++)
    for (j = 0; j < BOMB_ARGUMENTS; j++)
      printf("bombs[%d][%d] = %d\n", i, j, bombs[i][j]);

}