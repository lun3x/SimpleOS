#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// Include functionality relating to the   kernel.

#include "lolevel.h"
#include     "int.h"

/* The kernel source code is made simpler via three type definitions:
 *
 * - a type that captures a Process IDentifier (PID), which is really
 *   just an integer,
 * - a type that captures each component of an execution context (i.e.,
 *   processor state) in a compatible order wrt. the low-level handler
 *   preservation and restoration prologue and epilogue, and
 * - a type that captures a process PCB.
 */

#define MAX_PROGS  20
#define MAX_PIPES  20
#define STACK_SIZE 0x00005000

typedef int pid_t;

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef enum {
  OPEN,
  CLOSED,
  CREATED,
  READY,
  EXECUTING,
  WAITING,
  TERMINATED
} status_t;

typedef struct {
  pid_t pid;
  ctx_t ctx;
  status_t status;
  int priority;
} pcb_t;

typedef struct {
  pid_t proc1;
  pid_t proc2;
  int value;
  int id;
  status_t status;
} pipe_t;

#endif
