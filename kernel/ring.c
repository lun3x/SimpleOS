#include "ring.h"

struct user { // User at a node
  char name[20];
  int user_id;
};

struct node { // Node in a linked list
  struct node *next; // Pointer to next node
  struct node *prev; // Pointer to previous node
  User *user; // User at node
};
typedef struct node Node;

struct ring {
  Node *first;
  Node *last;
  Node *current;
};

char *get_name(Ring *ring) {
  return ring->current->user->name;
}

int get_id(Ring *ring) {
  return ring->current->user->user_id;
}

void set_name(User *user, char name[20]) {
  strcpy(user->name, name);
}

void set_id(User *user, int id) {
  user->user_id = id;
}

User *create_user(char name[20], int id) {
  User *new_user = malloc(sizeof(User));
  strcpy(new_user->name, name);
  new_user->user_id = id;

  return new_user;
}

void set_first(Ring *ring) {
  ring->current = ring->first;
  move_fwd(ring);
}

void set_last(Ring *ring) {
  ring->current = ring->last;
  move_back(ring);
}

User *get_user(Ring *ring) {
  return ring->current->user;
}

void move_fwd(Ring *ring) {
  ring->current = ring->current->next;
}

void move_back(Ring *ring) {
  ring->current = ring->current->prev;
}

Node *create_node(User *user, Node *next_node, Node *prev_node) {
  Node *new_node = malloc(sizeof(Node));
  new_node->next = next_node;
  new_node->prev = prev_node;
  new_node->user = user;

  return new_node;
}

void insert_before(Ring *ring, User *user) {
  Node *new_node = create_node(user, ring->current, ring->current->prev);
  ring->current->prev->next = new_node;
  ring->current->prev = new_node;
}

void insert_after(Ring *ring, User *user) {
  Node *new_node = create_node(user, ring->current->next, ring->current);
  ring->current->next->prev = new_node;
  ring->current->next = new_node;
}

Ring *create_ring() {
  Ring *new_ring = malloc(sizeof(Ring));

  Node *sentinel_node = create_node(NULL, NULL, NULL);
  sentinel_node->next = sentinel_node;
  sentinel_node->prev = sentinel_node;

  new_ring->first = sentinel_node;
  new_ring->last = sentinel_node;
  new_ring->current = sentinel_node;

  return new_ring;
}

bool is_sentinel(Node *node) {
  if (node->user == NULL) {
    return true;
  }
  else {
    return false;
  }
}

void delete(Ring *ring) {
  if (!is_sentinel(ring->current)) {
    Node *new_current;
    ring->current->next->prev = ring->current->prev;
    ring->current->prev->next = ring->current->next;
    new_current = ring->current;
    free(ring->current);
    ring->current = new_current->next;
  }
}

void print_ring(Ring *ring, int max_num_to_print) {
  set_last(ring);
  int i = 1;
  while (!is_sentinel(ring->current) && i <= max_num_to_print) {
    //printf("      %d: (%d, %s)\n", i, get_id(ring), get_name(ring));
    move_back(ring);
    i++;
  }
}

int locate_by_id(Ring *ring, int id) {
  set_last(ring);
  while (!is_sentinel(ring->current)) {
    if (ring->current->user->user_id == id) {
      return 1;
    }
    move_back(ring);
  }
  return 0;
}

void move_to_end(Ring *ring) {
  Node *node_to_be_moved = ring->current;
  delete(ring);
  set_last(ring);
  insert_after(ring, node_to_be_moved->user);
}

void error(int value) {
  if (value == 1) {
    //printf("Non existant user!\n");
  }
  else if (value == 2) {
    //printf("Incorrect user name for that ID!\n");
  }
  else if (value == 3) {
    //printf("Invalid user ID!\n");
  }
  else {
    //printf("Unknown error!\n");
  }
}

int get_ring_length(Ring *ring) {
  int length = 0;
  set_last(ring);
  while (!is_sentinel(ring->current)) {
    length++;
    move_back(ring);
  }
  return length;
}
