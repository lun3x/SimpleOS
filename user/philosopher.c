#include "philosopher.h"

void main_philosopher() {
  int current_pid = get_proc_id();
  // assume console is 1st prog, waiter is 2nd
  int philosopher_id = current_pid - 3;

  int can_eat;
  while (1) {
    can_eat = read_pipe(philosopher_id);

    char phil_id_str[2];
    itoa(phil_id_str, philosopher_id + 1);
    write(STDOUT_FILENO, phil_id_str, 2);

    if (can_eat) {
      write(STDOUT_FILENO, ": eating  \n", 12);
    }
    else {
      write(STDOUT_FILENO, ": thinking\n", 12);
    }
  }

  exit( EXIT_SUCCESS );
}
