#ifndef __RING_H
#define __RING_H

#include "hilevel.h"

typedef struct node { // Node in a linked list
  struct node *next;  // Pointer to next node
  struct node *prev;  // Pointer to previous node
  void *item;         // item at node
} Node;

// The type of a ring. Only the ring module has access to the details.
typedef struct ring {
  Node *first;
  Node *last;
  Node *current;
} Ring;

ctx_t *create_ctx(uint32_t cpsr, uint32_t pc, uint32_t sp);

// Return a pointer to a new pcb with given pcb id and pcb name.
pcb_t *create_pcb(pid_t pid, int priority, ctx_t *ctx);

// Set the pointer to the current pcb, to the first pcb in the ring.
void set_first(Ring *ring);

// Set the pointer to the current pcb, to the last pcb in the ring.
void set_last(Ring *ring);

pid_t get_current_pipe_id(Ring *ring);

pipe_t *get_current_pipe(Ring *ring);

// Return the pcb at the current position in the ring.
pcb_t *get_current_process(Ring *ring);

// Move the current pointer forward one position in the ring.
void move_fwd(Ring *ring);

// Move the current pointer forward one position in the ring.
void move_back(Ring *ring);

// Insert a new node containing a given pcb before the current position in the list.
void insert_before(Ring *ring, void *item);

// Insert a new node containing a given pcb after the current position in the list.
void insert_after(Ring *ring, void *item);

// Return a new empty ring, containing only the sentinel node.
Ring *create_ring();

// Delete the current node, moving the current pointer forward to the next pcb.
// Don't do anything if the current node is the sentinel node.
void delete(Ring *ring);

// Print the given ring up to the maximum number of entries given.
void print_ring(Ring *ring, int max_num_to_print);

// Sets the current pointer to the node containing the pcb with the given id.
// Return either 1 or 0 if the operation is successful or not respectively.
int locate_by_id(Ring *ring, pid_t id);

int locate_by_pipe_id(Ring *ring, pid_t id);

// Move the node at the current pointer to the end of the ring.
void move_to_end(Ring *ring);

// Prints error message corresponding to error value.
void error(int value);

// Returns the total number of entries in a ring.
int get_ring_length(Ring *ring);

// Moves current to next pid
void locate_next_pid(Ring *ring);

// Returns the pid of the current program in the ring
pid_t get_current_pid(Ring *ring);

// Returns the distance from the start of the ring to the current pcb
int get_current_pos_in_ring(Ring *ring);

// Increments priority of all processes except current
void age_processes(Ring *ring);

// Moves current to highest priority pcb
void locate_highest_priority(Ring *ring);

// Returns the largest pid in use
int get_max_pid(Ring *ring);

// Returns the largest pipe_id in use
int get_max_pipe_id(Ring *ring);

#endif
