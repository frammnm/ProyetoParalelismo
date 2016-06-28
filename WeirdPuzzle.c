

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include "mpi.h"

int  N, W; 
char *puzzle;
char *words; 


int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

int search_row(char array[], char word[]){
  int i, p, w;

  for (p=0;p<N;p++){
    if (search_lr(p,array,word)) return 1;
  }
  for (p=0 ; p < N ;p++){
    if (search_rl(p,array,word)) return 1;
  }
  return 0;
}

int search_lr(int pos,char array[], char word[]){
    int w, i;
    
    w=0; 
    for(i=pos;i < pos + N;i++){
      // printf("a[%d]:%c word[%d]:%c \n",i%N,array[i%N],w,word[w]);
      if(array[i%N] == word[w]){
        w++;
      }
      if (w == strlen(word)-1) return 1; 
    }
    return 0;
}

int process_file(char *name) {

  FILE *fp;
  char line[1024];
  int c, i, j, pos;

  if ((fp = fopen(name, "r")) == NULL) {
    printf("The input file \"%s\" doesn't exist!\n", name);
    return -1;
  };

  c = 0;
  pos = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (c == 0) {
      N = atoi(line);
      puzzle = (char *) malloc(sizeof(char) * N * N);
    }
    else if (c < N + 1) {
      pos = process_line(puzzle, line, pos);
    } 
    else if (c == N + 1) {
      W = atoi(line);
      words = (char *) malloc(sizeof(char) * N * N);
      pos = 0;
    } 
    else {
      pos = process_line(words, line, pos); 
    } 
    c++;
  };

}

int process_line(char array[], char *line, int pos) {
  int i = pos;
  int j = 0;
  int k = 0;
  char string[1024];

  strcpy(string,line);

  while (i < pos + N ){
      array[i] = string[j];
      i++;
      j++;
  };
  return i;
}

int search_rl(int pos,char array[], char word[]){
    int w, i;
    w=0; 
    for(i=pos;i > pos - N + 1 ;i--){
      if(array[mod(i,4)] == word[w]){
        w++;
      }
      if (w == strlen(word)-1) return 1; 
    }
    return 0;
}

int main(int argc, char *argv[]) {
  int i, j;
  int me, numProcessors, root = 0;

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  if (process_file(argv[1]) == -1)
    return 0;

  //Inicializacion de MPI
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &me);
  MPI_Comm_size (MPI_COMM_WORLD, &numProcessors);

  //Se calculan los rangos a enviar para cada esclavo como filas
  int sendcounts[numProcessors];
  int quotient = N / numProcessors;
  int rm = N % numProcessors;
  int displs[numProcessors];
  int sum = 0;
  
  for (i = 0; i < numProcessors; i++) {
    if (rm > 0) sendcounts[i] = (quotient + 1)*N;
    else sendcounts[i] = quotient*N;
    rm--;
    displs[i] = sum;
    sum += sendcounts[i];
  }

  //Se envian las filas correspondientes a cada proceso
  char *recv_rows = (char *)malloc(sizeof(char) * N * sendcounts[me]);
  MPI_Scatterv(puzzle, sendcounts, displs, MPI_CHAR, 
               recv_rows, sendcounts[me], MPI_CHAR, root, 
               MPI_COMM_WORLD);

  //Se calculan los rangos a enviar para cada esclavo como columnas
  int sendcountsC[numProcessors];
  int quotientC = N / numProcessors;
  int rmC = N % numProcessors;
  int displsC[numProcessors];
  int sumC = 0;

  for (i = 0; i < numProcessors; i++) {
    if (rm > 0) sendcountsC[i] = (quotientC + 1);
    else sendcountsC[i] = quotientC;
    rmC--;
    displsC[i] = sumC;
    sumC += sendcountsC[i];
  }
  
  //Se crean los tipos de columnas necesarios
  MPI_Datatype column_type,ncolumn_type;
  MPI_Type_vector(N,1,N,MPI_CHAR,&column_type);
  MPI_Type_commit(&column_type);

  MPI_Datatype columnb_type,ncolumnb_type;
  MPI_Type_vector(N,1,sendcountsC[me],MPI_CHAR,&columnb_type);
  MPI_Type_commit(&columnb_type);
  MPI_Type_create_resized(column_type, 0, 1*sizeof(char), &ncolumn_type);
  MPI_Type_create_resized(columnb_type, 0, 1*sizeof(char), &ncolumnb_type);
  MPI_Type_commit(&ncolumnb_type);
  MPI_Type_commit(&ncolumn_type);

  //Se envia la informacion a cada nodo
  char *recv_cols = (char *)malloc(sizeof(char) * N * sendcountsC[me]);
  MPI_Scatterv(puzzle, sendcountsC, displsC, ncolumn_type, 
               recv_cols, sendcountsC[me], ncolumnb_type, root, 
               MPI_COMM_WORLD);
  
  int sum_rows = 0; 
  j = 0;
  char r[N];
  char w[N];
  for (i=0;i<sendcounts[me]/N;i++){
    for (j=0;j<W;j++){
      if (search_row(&recv_rows[i*N],&words[j*N]) == 1){
        sum_rows+=1;
        break;
      }
    }
  }

  int sum_cols = 0; 
  for (i=0;i<sendcountsC[me];i++){
    for (j=0;j<W;j++){
      if (search_row(&recv_cols[i*N],&words[j*N]) == 1){
        sum_cols+=1;
        break;
      }
    }
  }

  int res = sum_rows+sum_cols;

  // Recolectar la suma de ocurrencias
  int *ress = NULL;
  if (me == root) {
    ress = (int *)malloc(sizeof(int) * numProcessors);
    assert(ress != NULL);
  }

  MPI_Gather(&res, 1, MPI_INT, ress, 1, MPI_INT, 0, MPI_COMM_WORLD);


  if (me==root) printf("Calculating final value..\n");
  //Calcular los resultados finales
  if (me==root){
    int final = 0;
    for(i=0;i<numProcessors;i++) final+=ress[i];  
    printf("Final:%d\n",final);
  }


  MPI_Finalize();
  return 0;


}
 

