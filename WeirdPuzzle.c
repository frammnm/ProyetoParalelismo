int  N, W; 
int *puzzle;
int *words; 

int main(int argc, char *argv[]) {
  int i, j;

  if (argc != 2) {
    printf("Invalid number of arguments!\n");
    return 0;
  }

  if (process_file(argv[1]) == -1)
    return 0;

  printf("N = %d\n", N);
  printf("T = %d\n", W);

  // printf("\n");
  // for (i = 0; i < T; i++) {
  //   for (j = 0; j < TARGET_ARGUMENTS; j++)
  //     printf("targets[%d][%d] = %d ", i, j, targets[TARGET_ARGUMENTS * i + j]);
  //   printf("\n");
  // }

  // printf("\n");
  // for (i = 0; i < B; i++) {
  //   for (j = 0; j < BOMB_ARGUMENTS; j++)
  //     printf("bombs[%d][%d] = %d ", i, j, bombs[BOMB_ARGUMENTS * i + j]);
  //   printf("\n");
  // }


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
  while (fgets(line, sizeof(line), fp)) {
    
    if (c == 0) {
      N = atoi(line);
      puzzle = (int *) malloc(sizeof(int) * N * N);
    }
    else if (c != N) {
      pos = process_line(puzzle, line, pos);
    } 
    else if (c == N ) {
        W = atoi(line);
        words = (int *) malloc(sizeof(int) * N * N);
    } 
    else {
        pos = process_words(words, line);
    } 
    c++;
  };

}


int process_line(int array[], char *line, int pos) {
  char *token;
  int i = pos;

  token = strtok(line, " ");
  array[i] = atoi(token);
  i++;

  while (token != NULL) {
    token = strtok(NULL, " ");
    array[i] = token;
    i++;
  }

  return i-1;
}

int process_words(int array[], char *line, int pos){



}