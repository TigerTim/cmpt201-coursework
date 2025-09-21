#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> // waitpid
#include <unistd.h>   // fork, execl

int main() {
  char *buff = NULL; // ptr of getline
  size_t size = 0;

  while (1) {
    printf("Enter programs to run.\n");
    fflush(stdout);

    ssize_t nread = getline(&buff, &size, stdin);
    if (nread == -1) {
      perror("getline");
      break;
    }

    // remove trailing newline
    if (buff[nread - 1] == '\n') {
      buff[nread - 1] = '\0';
    }

    // ignore empty input
    if (strlen(buff) == 0) {
      continue;
    }

    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      continue;
    }

    if (pid == 0) {
      // child process: replace w new program
      execl(buff, buff, (char *)NULL);

      // if execl returns, it failed
      fprintf(stderr, "Exec failure\n");
      exit(EXIT_FAILURE);
    } else {
      // parent process: wait for child
      int status;
      if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
      }
    }
  }

  free(buff);
  return 0;
}
