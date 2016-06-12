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
//#include "mpi.h"

int N, T, B;
int *targets;
int *bombs;
int **battleField;

char* LastcharDel(char*);
int process_line(int array[], char *line, int pos);
int process_file(char *name);

/*
###############################################################################
#############################         main         ############################
###############################################################################
*/
int main(int argc, char *argv[]) {
  int i, j;
  int yo, numProcesos;

  printf("hola\n");

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  if (process_file(argv[1]) == -1)
    return 0;

  // printf("N = %d\n", N);
  // printf("T = %d\n", T);
  // printf("B = %d\n", B);

  // battleField = (int **) malloc(sizeof(int *) * N);
  // assert(battleField != NULL);

  // for (i = 0; i < N; i++) {
  //   battleField[i] = (int *) malloc(sizeof(int) * N);
  //   assert(battleField[i] != NULL);
  //   for (j = 0; j < N; j++)
  //     battleField[i][j] = 0;
  // }

  // for (i = 0; i < T; i++) {
  //   battleField[targets[TARGET_ARGUMENTS * i]][targets[TARGET_ARGUMENTS * i + 1]] = targets[TARGET_ARGUMENTS * i + 2];
  // }

  return 0;

}


char* LastcharDel(char* string) {
    int i = 0;
    while(string[i] != '\0') {
        if (string[i] == '\n')
          string[i] = '\0';
        i++;
    }
    return string;
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
  n = strlen(token);
  // printf("%d ", n);
  // printf("%s\n", token);

  while (token != NULL) {
    token = strtok(NULL, " ");
    n = strlen(token);
    string = (char *) malloc(sizeof(char));
    *string = token[0];
    array[i] = atoi(string);
    i++;
  }

  return i-1;
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
    printf("ajajaja\n");
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
