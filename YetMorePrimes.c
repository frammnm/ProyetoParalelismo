/* Archivo: YetMorePrimes.c
 * Contiene toda la funcionalidad para el ejercicio C del proyecto.
 * Autores: 
   - Francisco Martinez  09-10502
   - Gabriel Alvarez     09-10029
*/ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"
#include <math.h>

int P;
char **firstHalf;
char **secondHalf;

void free_array_of_strings(char **strings);
int is_prime(int number);
void generate_primes(int info[2], int me, int *primes);
void distribute_work(int info[2], int numProcessors, int me);
int process_file(char *name);

/*
###############################################################################
#############################         main         ############################
###############################################################################
*/
int main(int argc, char *argv[]) {
  int i, j, k, root = 0;
  int me, numProcessors;
  MPI_Status status;
  int info[2];
  int *primes;
  int generated_primes[P];

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

  if (P < numProcessors) {
    printf("The number of primes is less than the number of processors, finalizing.\n");
    MPI_Finalize();
    return 0;
  }

  if (me == root) { 
    printf("firstHalf:\n");
    for (i = 0; i < P; i++) {
      printf("%s\n", firstHalf[i]);
    }

    printf("secondHalf:\n");
    for (i = 0; i < P; i++) {
      printf("%s\n", secondHalf[i]);
    }
    printf("\n");
    distribute_work(info, numProcessors, me);
  } else {
    MPI_Recv(info, 2, MPI_INT, root, 1, MPI_COMM_WORLD, &status);
  }

  primes = (int *) malloc(sizeof(int) * P);
  assert(primes != NULL);

  for (i = 0; i < P; i++)
    primes[i] = 0;

  generate_primes(info, me, primes);

  if (me == root) {
    int recv_primes[P];

    k = 0;
    for (i = 0; i < P; i++)
      if (primes[i] != 0) {
        generated_primes[k] = primes[i];
        k++;
      };
    for (i = 1; i < numProcessors; i++) {
      MPI_Recv(recv_primes, P, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
      for (i = 0; i < P; i++)
        if (recv_primes[i] != 0) {
          generated_primes[k] = recv_primes[i];
          k++;
        };
    }

    for (i = 0; i < P; i++)
      printf("%d\n", generated_primes[i]);
  }

  free(primes);
  free_array_of_strings(firstHalf);
  free_array_of_strings(secondHalf);
  MPI_Finalize();

}

/*
###############################################################################
###########################  free_array_of_strings  ###########################
###############################################################################
*/
void free_array_of_strings(char **strings) {
  int i;

  for (i = 0; i < P; i++)
    free(strings[i]);
  free(strings);
}

/*
###############################################################################
############################        is_prime        ###########################
###############################################################################
*/
int is_prime(int number) {
  int i;
  int square_root = (int) sqrt((double) number);

  for (i = 2; i <= square_root; i++)
    if (number % i == 0) return 0;
  return 1;
}

/*
###############################################################################
############################    generate_primes     ###########################
###############################################################################
*/
void generate_primes(int info[2], int me, int *primes) {
  int i, j, c, prime;

  c = 0;
  for (i = info[0]; i < info[0] + info[1]; i++) {
    for (j = 0; j < P; j++) {
      char *string = (char *) malloc(sizeof(char) * 256);
      assert(string != NULL);
      strcpy(string, firstHalf[i]);
      strcat(string, secondHalf[j]);
      prime = atoi(string);
      free(string);
      if (is_prime(prime) == 1) {
        primes[c] = prime;
        c++;
        break;
      };
    }
  }
  if (me != 0) MPI_Send(primes, P, MPI_INT, 0, 1, MPI_COMM_WORLD);
}

/*
###############################################################################
#############################    distribute_work   ############################
###############################################################################
*/
void distribute_work(int info[2], int numProcessors, int me) {
  int primesPerProcessor[numProcessors];
  int i = 0;
  int pos = 0;
  int send[2];
  int quotient = P / numProcessors;
  int rm = P % numProcessors;

  for (i = 0; i < numProcessors; i++) {
    if (rm > 0) primesPerProcessor[i] = quotient + 1;
    else primesPerProcessor[i] = quotient;
    rm--;
    if (i == 0) {
      info[0] = pos;
      info[1] = primesPerProcessor[i];
    } else {
      send[0] = pos;
      send[1] = primesPerProcessor[i];
      MPI_Send(send, 2, MPI_INT, i, 1, MPI_COMM_WORLD);
    }
    pos += primesPerProcessor[i];
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
    if (c == 0) {
      P = atoi(line);
      firstHalf = (char **) malloc(sizeof(char *) * P);
      secondHalf = (char **) malloc(sizeof(char *) * P);
      assert(firstHalf != NULL);
      assert(secondHalf != NULL);
      i = 0;
    } else if (c < P) {
      j = atoi(line);
      firstHalf[i] = (char *) malloc(sizeof(char) * 256);
      assert(firstHalf[i] != NULL);
      asprintf(&firstHalf[i], "%i", j);
      i++;
    } else if (c == P) {
      j = atoi(line);
      firstHalf[i] = (char *) malloc(sizeof(char) * 256);
      assert(firstHalf[i] != NULL);
      asprintf(&firstHalf[i], "%i", j);
      i = 0;
    } else {
      j = atoi(line);
      secondHalf[i] = (char *) malloc(sizeof(char) * 256);
      assert(secondHalf[i] != NULL);
      if (line[0] == '0') {
        char *string = (char *) malloc(sizeof(char) * 256);
        assert(string != NULL);
        asprintf(&string, "%i", j);
        secondHalf[i][0] = '0';
        strcat(secondHalf[i], string);
        free(string);
      } else {
        asprintf(&secondHalf[i], "%i", j);
      }

      i++;
    }
    c++;
  };

  fclose(fp);
  return 0;
}