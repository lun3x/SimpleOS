#include "waiter.h"

extern void main_philosopher();

pid_t philosopher_pids[NUM_PHILOSOPHERS];
int philosopher_pipe_ids[NUM_PHILOSOPHERS];

void main_waiter() {
  pid_t proc_id = get_proc_id();

  for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
    philosopher_pipe_ids[i] = open_pipe( (proc_id + 1) + i, (proc_id + 1) + ((i + 1) % NUM_PHILOSOPHERS) );

    philosopher_pids[i] = fork(10);

    if (philosopher_pids[i] == 0) {
      exec( &main_philosopher );
    }
  }

  write( STDOUT_FILENO, "end waiter", 10 );

  exit( EXIT_SUCCESS );
}
