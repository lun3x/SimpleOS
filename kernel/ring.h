#ifndef __RING_H
#define __RING_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// The type of a ring. Only the ring module has access to the details.
struct ring;
typedef struct ring Ring;

// The type of user. Only the ring module has access to the details.
struct user;
typedef struct user User;

// Get the user id of the user at the current location.
int get_id(Ring *ring);

// Get the user name of the user at the current location.
char *get_name(Ring *ring);

// Set the user name of the given user to the given string.
void set_name(User *user, char name[20]);

// Set the id of the given user to the given id.
void set_id(User *user, int id);

// Return a pointer to a new user with given user id and user name.
User *create_user(char name[20], int id);

// Set the pointer to the current user, to the first user in the ring.
void set_first(Ring *ring);

// Set the pointer to the current user, to the last user in the ring.
void set_last(Ring *ring);

// Return the user at the current position in the ring.
User *get_user(Ring *ring);

// Move the current pointer forward one position in the ring.
void move_fwd(Ring *ring);

// Move the current pointer forward one position in the ring.
void move_back(Ring *ring);

// Insert a new node containing a given user before the current position in the list.
void insert_before(Ring *ring, User *user);

// Insert a new node containing a given user after the current position in the list.
void insert_after(Ring *ring, User *user);

// Return a new empty ring, containing only the sentinel node.
Ring *create_ring();

// Delete the current node, moving the current pointer forward to the next user.
// Don't do anything if the current node is the sentinel node.
void delete(Ring *ring);

// Print the given ring up to the maximum number of entries given.
void print_ring(Ring *ring, int max_num_to_print);

// Sets the current pointer to the node containing the user with the given id.
// Return either 1 or 0 if the operation is successful or not respectively.
int locate_by_id(Ring *ring, int id);

// Move the node at the current pointer to the end of the ring.
void move_to_end(Ring *ring);

// Prints error message corresponding to error value.
void error(int value);

// Returns the total number of entries in a ring.
int get_ring_length(Ring *ring);

#endif
