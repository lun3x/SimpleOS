#include "philosopher.h"

void main_philosopher() {
  int current_pid = get_proc_id();
  int philosopher_id = current_pid - 2;
  int num_chopsticks = 0;

  // int left_pipe_id = open_pipe(philosopher_id + 2, ((philosopher_id + 1) % NUM_PHILOSOPHERS) + 2);

  while (1) {
    if (philosopher_id != NUM_PHILOSOPHERS - 1) {       // for most philosophers
      int lower_stick = read_pipe(philosopher_id);      // check lower numbered pipe

      if (lower_stick == 0) {                           // if 0, set to 1, and increment num chopsticks
        write_pipe(philosopher_id, 1);
        num_chopsticks++;
      }

      int higher_stick = read_pipe(philosopher_id + 1); // check higher numbered pipe

      if (higher_stick == 0) {
        write_pipe(philosopher_id + 1, 1);              // if 0, set to 1 and increment num chopsticks
        num_chopsticks++;
      }

      if (num_chopsticks == 2) {
        write( STDOUT_FILENO, "  eating  ", 10 );
        write_pipe(philosopher_id, 0);
        write_pipe(philosopher_id + 1, 0);
        num_chopsticks = 0;
      }
      else {
        write( STDOUT_FILENO, " thinking ", 10 );
      }
    }
    else {                                             // for last philosopher
      int lower_stick = read_pipe(0);                  // check lower numbered pipe

      if (lower_stick == 0) {                          // if 0, set to 1, and increment num chopsticks
        write_pipe(philosopher_id, 1);
        num_chopsticks++;
      }

      int higher_stick = read_pipe(philosopher_id);     // check higher numbered pipe

      if (higher_stick == 0) {                          // if 0, set to 1 and increment num chopsticks
        write_pipe(philosopher_id + 1, 1);
        num_chopsticks++;
      }

      if (num_chopsticks == 2) {                        // if philosopher has both chopsticks, write eating, then put down both chopsticks
        write( STDOUT_FILENO, "  eating  ", 10 );
        write_pipe(philosopher_id, 0);
        write_pipe(0, 0);
        num_chopsticks = 0;
      }
      else {
        write( STDOUT_FILENO, " thinking ", 10 );
      }
    }
    yield();
  }

  exit( EXIT_SUCCESS );
}
