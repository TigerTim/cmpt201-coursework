#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 128

// 2 128-byte blocks (header size inclusive) and each has a header
struct header {
  uint64_t size;
  struct header *next;
};

void handle_error(const char *msg) {
  write(STDERR_FILENO, msg, strlen(msg));
  write(STDERR_FILENO, "\n", 1);
  _exit(1);
}

void print_out(char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len = snprintf(buf, BUF_SIZE, format,
                         data_size == sizeof(uint64_t) ? *(uint64_t *)data
                                                       : *(void **)data);

  if (len < 0) {
    handle_error("snprintf");
  }

  write(STDOUT_FILENO, buf, len);
}

int main() {
  // 1. Increase heap by 256 bytes
  void *start =
      sbrk(256); // the return value of sbrk is the value b4 the function call
  if (start == (void *)-1)
    handle_error("sbrk failed");

  // 2. define block addr
  void *block1_addr =
      start; // start has value of program break b4 sbrk() -> first pos on heap
  void *block2_addr = (void *)((char *)start + 128); // each block is 128 bytes

  // 3.  create headers for each block
  struct header *block1 = (struct header *)block1_addr;
  struct header *block2 = (struct header *)block2_addr;

  block1->size = 128;
  block1->next = NULL;

  block2->size = 128;
  block2->next = block1;

  // 4. print initial headers
  print_out("Block1 start: %p\n", &block1_addr, sizeof(block1_addr));
  print_out("Block2 start: %p\n", &block2_addr, sizeof(block2_addr));
  print_out("Block1 size: %lu\n", &block1->size, sizeof(block1->size));
  print_out("Block1 next: %p\n", &block1->next, sizeof(block1->next));
  print_out("Block2 size: %lu\n", &block2->size, sizeof(block2->size));
  print_out("Block2 next: %p\n", &block2->next, sizeof(block2->next));

  // 5. initialize data regions
  void *data1 = (void *)((char *)block1_addr + sizeof(struct header));
  void *data2 = (void *)((char *)block2_addr + sizeof(struct header));
  size_t data_size = 128 - sizeof(struct header);

  memset(data1, 0, data_size);
  memset(data2, 1, data_size);

  // 6. print each byte in data sections
  write(STDOUT_FILENO, "Block1 data bytes:\n", 20);
  for (size_t i = 0; i < data_size; i++) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", ((unsigned char *)data1)[i]);
    write(STDOUT_FILENO, buf, len);
  }

  write(STDOUT_FILENO, "\n", 1);

  write(STDOUT_FILENO, "Block2 data bytes:\n", 20);
  for (size_t i = 0; i < data_size; i++) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", ((unsigned char *)data2)[i]);
    write(STDOUT_FILENO, buf, len);
  }

  write(STDOUT_FILENO, "\n", 1);
  return 0;
}
