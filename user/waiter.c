#include "waiter.h"

philosopher_pipe_ids[PHILOSOPHER_NUM];

void main_waiter() {
  pid_t proc_id = get_proc_id();

  // loop over fork to create correct number of child processes
  for (int i = 0; i < PHILOSOPHER_NUM; i++) {
    fork(10);

    philosopher_pipe_ids[i] = open_pipe(proc_id, philosopher_pids[i]);

    if (philosopher_pids[i] == 0) {
      exec(&main_philosopher);
    }
  }

  // one philosopher eats, the rest think
  int eating_philosopher = 0;
  while (1) {
    for (int i = 0; i < PHILOSOPHER_NUM; i++) {
      if (i == eating_philosopher) write_pipe(i, 1);
      else                         write_pipe(i, 0);
    }
    eating_philosopher = (eating_philosopher + 1) % PHILOSOPHER_NUM;
  }

  exit( EXIT_SUCCESS );
}
