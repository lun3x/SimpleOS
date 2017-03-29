#include "philosopher.h"

void main_philosopher() {
  int current_pid = get_proc_id();
  int philosopher_id = current_pid - 2;
  int num_chopsticks = 0;

  while (1) {
    if (philosopher_id != NUM_PHILOSOPHERS - 1) {       // for most philosophers
      int lower_stick = read_pipe(philosopher_id);      // check lower numbered pipe

      if (lower_stick == 0) {                           // if 0, set to 1, and increment num chopsticks
        write_pipe(philosopher_id, 1);
        num_chopsticks++;
      }

      int higher_stick = read_pipe(philosopher_id + 1); // check higher numbered pipe

      if (higher_stick == 0) {                          // if 0, set to 1 and increment num chopsticks
        write_pipe(philosopher_id + 1, 1);
        num_chopsticks++;
      }

      if (num_chopsticks == 2) {
        char phil_id_str[1];
        itoa(phil_id_str, philosopher_id);
        write( STDOUT_FILENO, phil_id_str, 1 );
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
        char phil_id_str[1];
        itoa(phil_id_str, philosopher_id);
        write( STDOUT_FILENO, phil_id_str, 1 );
        write_pipe(philosopher_id, 0);
        write_pipe(0, 0);
        num_chopsticks = 0;
      }
      else {
        write( STDOUT_FILENO, " thinking ", 10 );
      }
    }
  }

  exit( EXIT_SUCCESS );
}
