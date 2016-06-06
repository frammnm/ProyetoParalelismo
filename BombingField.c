

#define TARGET_ARGUMENTS 3
#define BOMB_ARGUMENTS 4

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
//#include "mpi.h"

main(int argc, char *argv[]) {

  FILE *fp;
  int N, i, T, B, c, k;
  int **targets;
  int **bombs;
  char line[256];

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  };

  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("The input file \"%s\" doesn't exists!\n", argv[1]);
    return 0;
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
        char *token;
        int j = 0;
        /* get the first token */
        token = strtok(line, " ");
        targets[i] = (int *) malloc(sizeof(int) * TARGET_ARGUMENTS);
        targets[i][j] = atoi(token);
        j++;
        /* walk through other tokens */
        while (token != NULL) {
          token = strtok(NULL, " ");
          targets[i][j] = atoi(token);
          j++;
        }
        i++;
      } else if (i == T) {
        k = 0;
        B = atoi(line);
        bombs = (int **) malloc(sizeof(int *) * B);
        i++;
      }
      else {
        char *token;
        int j = 0;
        /* get the first token */
        token = strtok(line, " ");
        bombs[k] = (int *) malloc(sizeof(int) * BOMB_ARGUMENTS);
        bombs[k][j] = atoi(token);
        j++;
        /* walk through other tokens */
        while (token != NULL) {
          token = strtok(NULL, " ");
          bombs[k][j] = atoi(token);
          j++;
        }
        k++;
      }
    }
    c++;
  };
  int j;
  for (i = 0; i < T; i++)
    for (j = 0; j < TARGET_ARGUMENTS; j++)
      printf("targets[%d][%d] = %d\n", i, j, targets[i][j]);

  for (i = 0; i < B; i++)
    for (j = 0; j < BOMB_ARGUMENTS; j++)
      printf("bombs[%d][%d] = %d\n", i, j, bombs[i][j]);

  printf("n = %d\n", N);
  fclose(fp);
  // while ((read = getline(&line, &len, fp)) != -1) {
  //       printf("Retrieved line of length %zu :\n", read);
  //       printf("%s", line);
  // }

  


}