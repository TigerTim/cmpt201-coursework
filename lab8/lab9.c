// client.c
/*
Questions to answer at top of client.c:
(You should not need to change the code in client.c)
1. What is the address of the server it is trying to connect to (IP address and
port number). ANSWER: IP address 127.0.0.1 (localhost) and port 8000
   - The IP is defined as ADDR "127.0.0.1" and used in inet_pton()
   - The port is defined as PORT 8000 and used in htons(PORT)

2. Is it UDP or TCP? How do you know?
   ANSWER: TCP
   - The socket is created with SOCK_STREAM, which indicates TCP
   - UDP would use SOCK_DGRAM
   - Also, connect() is used, which is a TCP operation

3. The client is going to send some data to the server. Where does it get this
data from? How can you tell in the code? ANSWER: From standard input
(keyboard/stdin)
   - The while loop reads from STDIN_FILENO using read(STDIN_FILENO, buf,
BUF_SIZE)
   - STDIN_FILENO is file descriptor 0, which is standard input

4. How does the client program end? How can you tell that in the code?
   ANSWER: The client ends when read() returns 1 or less bytes from stdin
   - The while loop condition checks (num_read = read(STDIN_FILENO, buf,
BUF_SIZE)) > 1
   - When the user sends EOF (Ctrl+D) or enters just a newline, num_read will be
<= 1
   - Then the loop exits, the socket is closed with close(sfd), and
exit(EXIT_SUCCESS) is called
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}

// server.c
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32
#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0;
int client_id_counter = 1;
// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;
struct client_info {
  int cfd;
  int client_id;
};
void *handle_client(void *arg) {
  struct client_info *client = (struct client_info *)arg;

  // Extract client socket file descriptor and client ID
  int cfd = client->cfd;
  int client_id = client->client_id;

  // Free the dynamically allocated client_info struct
  free(client);

  // Buffer to read messages into
  char buf[BUF_SIZE];
  ssize_t num_read;

  // Loop to continuously read messages from the client
  while ((num_read = read(cfd, buf, BUF_SIZE)) > 0) {
    // Null-terminate the buffer to treat it as a string
    buf[num_read] = '\0';

    // Increment total message count (thread-safe)
    pthread_mutex_lock(&count_mutex);
    total_message_count++;
    int current_count = total_message_count;
    pthread_mutex_unlock(&count_mutex);

    // Print the message received
    printf("Msg # %3d; Client ID %d: %s", current_count, client_id, buf);
  }

  // When read returns 0 or -1, client has closed the connection
  printf("Ending thread for client %d\n", client_id);

  // Close the server's socket for this client
  close(cfd);

  return NULL;
}
int main() {
  struct sockaddr_in addr;
  int sfd;
  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }
  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }
  for (;;) {
    // Accept new client connection
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
      perror("accept");
      continue;
    }

    // Assign a client ID (thread-safe)
    pthread_mutex_lock(&client_id_mutex);
    int current_client_id = client_id_counter;
    client_id_counter++;
    pthread_mutex_unlock(&client_id_mutex);

    // Print message about new client
    printf("New client created! ID %d on socket FD %d\n", current_client_id,
           cfd);

    // Allocate memory for client info to pass to thread
    struct client_info *client = malloc(sizeof(struct client_info));
    if (client == NULL) {
      perror("malloc");
      close(cfd);
      continue;
    }
    client->cfd = cfd;
    client->client_id = current_client_id;

    // Create a new thread to handle this client
    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, client) != 0) {
      perror("pthread_create");
      free(client);
      close(cfd);
      continue;
    }

    // Detach the thread so it cleans up automatically when done
    pthread_detach(thread);
  }
  if (close(sfd) == -1) {
    handle_error("close");
  }
  return 0;
}
