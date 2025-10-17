#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(expr)                                                           \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                        \
      exit(1);                                                                 \
    }                                                                          \
  }

#define TEST(expr)                                                             \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Test failed: %s\n", #expr);                             \
      exit(1);                                                                 \
    }                                                                          \
  }

typedef struct node {
  uint64_t data;
  struct node *next;
} node_t;

typedef struct info {
  uint64_t sum;
} info_t;

node_t *head = NULL;
info_t info = {0};

void insert_sorted(uint64_t data) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->data = data;
  new_node->next = NULL;

  if (head == NULL) {
    head = new_node;
  } else if (data < head->data) {
    new_node->next = head;
    head = new_node;
  } else {
    node_t *curr = head;
    node_t *prev = NULL;

    while (curr != NULL) {
      if (data < curr->data) {
        break;
      }

      prev = curr;
      curr = curr->next;
    }

    prev->next = new_node;
    if (curr != NULL) {
      new_node->next = curr;
    }
  }

  info.sum += data;
}

int index_of(uint64_t data) {
  node_t *curr = head;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }

    curr = curr->next;
    index++;
  }

  return -1;
}

uint64_t calc_list_sum(node_t *head) {
  uint64_t sum = 0;
  node_t *current = head;
  while (current != NULL) {
    sum += current->data;
    current = current->next;
  }

  return sum;
}

int main() {
  insert_sorted(1);
  ASSERT(info.sum == calc_list_sum(head));
  insert_sorted(3);
  ASSERT(info.sum == calc_list_sum(head));
  insert_sorted(5);
  ASSERT(info.sum == calc_list_sum(head));
  insert_sorted(2);
  ASSERT(info.sum == calc_list_sum(head));

  TEST(info.sum == 1 + 3 + 5 + 2);
  TEST(index_of(2) == 1);
  printf("All tests passed!\n");
  return 0;
}
