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
