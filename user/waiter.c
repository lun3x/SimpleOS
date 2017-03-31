#include "waiter.h"

extern void main_philosopher();

pid_t philosopher_pids[PROBLEM_NUM];
int philosopher_pipe_ids[PROBLEM_NUM];

void main_waiter() {
  int solution_type = 0;
  pid_t proc_id = get_proc_id();

  for (int i = 0; i < PROBLEM_NUM; i++) {
    philosopher_pids[i] = fork(10);

    philosopher_pipe_ids[i] = open_pipe( proc_id,  philosopher_pids[i] );

    if (philosopher_pids[i] == 0) {
      exec( &main_philosopher );
    }
  }

  // while (1) {
  //   for (int j = 0; j < 2; j++) {
  //     for (int i = 0; i < PROBLEM_NUM; i++) {
  //       if (j == 0) {
  //         if (i%2 == 0) write_pipe(i, 0);
  //         if (i%2 == 1) write_pipe(i, 1);
  //       } else {
  //         if (i%2 == 0) write_pipe(i, 1);
  //         if (i%2 == 1) write_pipe(i, 0);
  //       }
  //     }
  //   }
  //   write( STDOUT_FILENO, "\n", 2 );
  //   yield();
  // }

  solution_type = PROBLEM_NUM % 2;

  if (solution_type == 0) {
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
  } else {
    int eating_philosopher = 0;

    while (1) {
      for (int i = 0; i < PROBLEM_NUM; i++) {
        if (i == eating_philosopher) write_pipe(i, 1);
        else                         write_pipe(i, 0);
      }

      eating_philosopher = (eating_philosopher + 1) % PROBLEM_NUM;

      write( STDOUT_FILENO, "\n", 2 );
    }
  }

  exit( EXIT_SUCCESS );
}
