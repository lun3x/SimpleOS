#include "waiter.h"

extern void main_philosopher();

pid_t philosopher_pids[PROBLEM_NUM];
int philosopher_pipe_ids[PROBLEM_NUM];

void main_waiter() {
  pid_t proc_id = get_proc_id();

  for (int i = 0; i < PROBLEM_NUM; i++) {
    philosopher_pids[i] = fork(10);

    philosopher_pipe_ids[i] = open_pipe( proc_id,  philosopher_pids[i] );

    if (philosopher_pids[i] == 0) {
      exec( &main_philosopher );
    }
  }

  while (1) {

    for (int i = 0; i < PROBLEM_NUM; i++) {
      if (i % 2 == 0) write_pipe(i, 0);
      else            write_pipe(i, 1);
    }

    write( STDOUT_FILENO, "\n", 2 );

    for (int i = 0; i < PROBLEM_NUM; i++) {
      if (i % 2 == 1) write_pipe(i, 0);
      else            write_pipe(i, 1);
    }

    write( STDOUT_FILENO, "\n", 2 );
  }

  exit( EXIT_SUCCESS );
}
