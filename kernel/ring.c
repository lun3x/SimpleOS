#include "ring.h"

// char *get_name(Ring *ring) {
//   return ring->current->pcb->name;
// }

// int get_id(Ring *ring) {
//   return ring->current->pcb->pid;
// }

// void set_name(pcb_t *pcb, char name[20]) {
//   strcpy(pcb->name, name);
// }

// void set_id(pcb_t *pcb, int id) {
//   pcb->pid = id;
// }

ctx_t *create_ctx(uint32_t cpsr, uint32_t pc, uint32_t sp) {
  ctx_t *new_ctx = malloc(sizeof(ctx_t));
  new_ctx->cpsr = cpsr;
  new_ctx->pc   = pc;
  new_ctx->sp   = sp;

  return new_ctx;
}

pcb_t *create_pcb(pid_t pid, int priority, status_t status, ctx_t *ctx) {
  pcb_t *new_pcb = malloc(sizeof(pcb_t));

  new_pcb->pid      = pid;
  new_pcb->priority = priority;
  new_pcb->status   = status;

  memcpy(&new_pcb->ctx, ctx, sizeof(ctx_t));

  return new_pcb;
}

void set_first(Ring *ring) {
  ring->current = ring->first;
  move_fwd(ring);
}

void set_last(Ring *ring) {
  ring->current = ring->last;
  move_back(ring);
}

pid_t get_current_pid(Ring *ring) {
  return ring->current->pcb->pid;
}

pcb_t *get_current_process(Ring *ring) {
  return ring->current->pcb;
}

void move_fwd(Ring *ring) {
  ring->current = ring->current->next;
}

void move_back(Ring *ring) {
  ring->current = ring->current->prev;
}

Node *create_node(pcb_t *pcb, Node *next_node, Node *prev_node) {
  Node *new_node = malloc(sizeof(Node));
  new_node->next = next_node;
  new_node->prev = prev_node;
  new_node->pcb = pcb;

  return new_node;
}

void insert_before(Ring *ring, pcb_t *pcb) {
  Node *new_node = create_node(pcb, ring->current, ring->current->prev);
  ring->current->prev->next = new_node;
  ring->current->prev = new_node;
}

void insert_after(Ring *ring, pcb_t *pcb) {
  Node *new_node = create_node(pcb, ring->current->next, ring->current);
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
  if (node->pcb == NULL) {
    return true;
  }
  else {
    return false;
  }
}

pid_t get_next_pid(Ring *ring) {
  if (!is_sentinel(ring->current->next)) { // if not at end of ring
    return ring->current->next->pcb->pid; // return next pid
  } else {
    return ring->first->next->pcb->pid; // else return first pid
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

int locate_by_id(Ring *ring, pid_t id) {
  set_last(ring);
  while (!is_sentinel(ring->current)) {
    if (ring->current->pcb->pid == id) {
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
  insert_after(ring, node_to_be_moved->pcb);
}

void error(int value) {
  if (value == 1) {
    //printf("Non existant pcb!\n");
  }
  else if (value == 2) {
    //printf("Incorrect pcb name for that ID!\n");
  }
  else if (value == 3) {
    //printf("Invalid pcb ID!\n");
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

int get_current_pos_in_ring(Ring *ring) {
  pid_t current_pid = get_current_pid(ring);
  int length = 0;
  while (!is_sentinel(ring->current)) {
    length++;
    move_back(ring);
  }
  int success = locate_by_id(ring, current_pid);
  return length;
}
