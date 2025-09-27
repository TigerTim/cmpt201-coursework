#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_history(char *history[], int index) {
  for (int i = 0; i < 5; i++) {
    int idx = (index + i) % 5;
    if (history[idx] != NULL) {
      printf("%s", history[idx]);
    }
  }
}

void add_to_history(char *history[], char *line, int *index) {
  if (history[*index] != NULL) {
    free(history[*index]);
  }

  history[*index] = line; // store ptr directly
  *index = (*index + 1) % 5;
}

int main() {
  char *history[5] = {NULL};
  int index = 0; // start running from index NOT always from 0
  char *line = NULL;
  size_t len = 0;

  while (1) {
    printf("Enter input: ");
    ssize_t read = getline(&line, &len, stdin);
    if (read == -1)
      break; // Ctrl+C

    if (strcmp(line, "print\n") == 0) {
      print_history(history, index);
      char *line_copy = strdup(line);
      add_to_history(history, line_copy, &index);
    }

    // case no "print" input
    else {
      add_to_history(history, line, &index);
      line = NULL; // getline alloc new mem
    }
  }

  // Free alloc mem
  for (int i = 0; i < 5; i++) {
    if (history[i] != NULL) {
      free(history[i]);
    }
  }

  free(line);
  return 0;
}
