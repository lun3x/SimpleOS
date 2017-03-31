#include "hilevel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user
 * programs, we can
 *
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */

// set of program control blocks, pointer to current pcb
//pcb_t pcb[ MAX_PROGS ], *current = NULL;

pipe_t pipes[MAX_PIPES];

Ring *pcb_ring;

// current maximum allocated program id
pid_t max_pid = 0;

// entry points for programs
extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_console();

// location of top of stack for user programs
extern uint32_t tos_user_progs;


// switch processes according to priority queue
void scheduler(ctx_t *ctx) {
  age_processes(pcb_ring);

  memcpy(&get_current_process(pcb_ring)->ctx,
         ctx,
         sizeof(ctx_t));

  locate_highest_priority(pcb_ring);

  memcpy(ctx,
         &get_current_process(pcb_ring)->ctx,
         sizeof(ctx_t));
}


// create new child process identical to parent
void hilevel_fork(ctx_t *ctx) {
  pcb_t *child_pcb = create_pcb(max_pid, ctx->gpr[0], EXECUTING, ctx);
  max_pid++;

  // insert new child pcb into ring after current pcb
  insert_after(pcb_ring, child_pcb);

  // find top of stack for for child and current program
  int new_tos     = (int) &tos_user_progs - child_pcb->pid            * STACK_SIZE;
  int current_tos = (int) &tos_user_progs - get_current_pid(pcb_ring) * STACK_SIZE;

  // find stack pointer location within stack
  int sp_location = current_tos - ctx->sp;

  // put child's stack pointer to correct place within stack
  child_pcb->ctx.sp = new_tos - sp_location;

  // copy across the current stack into the new stack
  memcpy((void *) new_tos - STACK_SIZE,
         (void *) current_tos - STACK_SIZE,
         STACK_SIZE);

  // return 0 to child process
  child_pcb->ctx.gpr[0] = 0;

  // return pid of child process to parent process
  ctx->gpr[0] = child_pcb->pid;

  // set status of new child process to EXECUTING
  child_pcb->status = EXECUTING;

  return;
}


// load new program image to be executed
void hilevel_exec(ctx_t* ctx) {
  // get current top of stack
  int current_tos = (int) &tos_user_progs - get_current_pid(pcb_ring) * STACK_SIZE;

  // initialise stack to zeros for security
  memset((void *) current_tos - STACK_SIZE,
         0,
         STACK_SIZE);

  // initialise stack pointer to start of stack
  ctx->sp = current_tos;

  // set pc to entry point of new function
  ctx->pc = ctx->gpr[0];

  // set gprs to 0 for security
  for (int i = 0; i < 13; i++) {
    ctx->gpr[i] = 0;
  }

  return;
}


// find program in pcb list and remove it
void hilevel_exit(ctx_t* ctx) {
  delete(pcb_ring);
}


// find index in pipes to put new pipe
int find_free_pipe_index() {
  for (int i = 0; i < MAX_PIPES; i++) {
    if (pipes[i].status == CLOSED) {
      return i;
    }
  }

  return -1;
}


// create a pipe
void hilevel_pipe_open(ctx_t *ctx) {
  int free_pipe_index = find_free_pipe_index();

  // if space for free pipes
  if (free_pipe_index >= 0) {
    pipes[ free_pipe_index ].proc1  = (pid_t) ctx->gpr[0];
    pipes[ free_pipe_index ].proc2  = (pid_t) ctx->gpr[1];
    pipes[ free_pipe_index ].value  = -1;
    pipes[ free_pipe_index ].id     = free_pipe_index;
    pipes[ free_pipe_index ].status = OPEN;
  }

  ctx->gpr[0] = free_pipe_index;

  return;
}


// write data to pipe
void hilevel_pipe_write( ctx_t *ctx ) {
  int pipe_id = ctx->gpr[0];
  int data    = ctx->gpr[1];

  if (pipes[ pipe_id ].status == OPEN) {
    // check program has permission to write to pipe
    if (pipes[ pipe_id ].proc1 == get_current_pid(pcb_ring) || pipes[ pipe_id ].proc2 == get_current_pid(pcb_ring)) {
      pipes[ pipe_id ].value = data;
    }
  }

  return;
}


//read data from pipe
void hilevel_pipe_read( ctx_t *ctx ) {
  int pipe_id = ctx->gpr[0];
  ctx->gpr[0] = -1;

  if (pipes[ pipe_id ].status == OPEN) {
    // check program has permission to read from pipe
    if (pipes[ pipe_id ].proc1 == get_current_pid(pcb_ring) || pipes[ pipe_id ].proc2 == get_current_pid(pcb_ring)) {

      // return value to calling function
      ctx->gpr[0] = pipes[ pipe_id ].value;

      // if overwrite flag on, reset pipe value to prevent multiple reads
      if (ctx->gpr[1]) {
        pipes[ pipe_id ].value = -1;
      }
    }
  }

  return;
}


// close existing pipe
void hilevel_pipe_close(ctx_t *ctx) {
  int pipe_id = ctx->gpr[0];

  if (pipes[pipe_id].status == OPEN) {
    // check program has permission to close pipe
    if (pipes[pipe_id].proc1 == get_current_pid(pcb_ring) || pipes[ pipe_id ].proc2 == get_current_pid(pcb_ring)) {
      pipes[pipe_id].value  = -1;
      pipes[pipe_id].status = CLOSED;
    }
  }
}


// get process id of current process
void hilevel_get_proc_id( ctx_t *ctx ) {
  // return pid to calling function
  ctx->gpr[0] = get_current_pid(pcb_ring);
  return;
}


// write to console
void hilevel_write( ctx_t *ctx ) {
  int   fd = ( int   )( ctx->gpr[ 0 ] );
  char*  x = ( char* )( ctx->gpr[ 1 ] );
  int    n = ( int   )( ctx->gpr[ 2 ] );

  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART0, *x++, true );
  }

  // return number of bits written to calling function
  ctx->gpr[ 0 ] = n;
}


// handle reset interrupt calls
void hilevel_handler_rst( ctx_t* ctx ) {
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  /*
   * - The CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - The PC and SP values match the entry point and top of stack.
   */
  max_pid = 0;

  pcb_ring = create_ring();
  // order = cpsr, pc, sp
  ctx_t *initial_ctx = create_ctx((uint32_t) 0x50, (uint32_t) &main_console, (uint32_t) &tos_user_progs);

  // set up initial process
  // order = pid, priority, status, ctx
  pcb_t *initial_pcb = create_pcb(max_pid, 10, EXECUTING, initial_ctx);
  max_pid++;

  insert_after(pcb_ring, initial_pcb);
  // set the current pointer to inital process
  set_first(pcb_ring);

  // copy the context of the initial program into the passed in context
  memcpy(ctx,
         &initial_pcb->ctx,
         sizeof(ctx_t));

  // initialise pipes to 0
  for (int i = 0; i < MAX_PIPES; i++) {
    memset( &pipes[i], 0, sizeof( pipe_t ) );
    pipes[i].status = CLOSED;
    pipes[i].id = -1;
  }

  int_enable_irq();

  return;
}


// handle irq interrupt calls
void hilevel_handler_irq(ctx_t* ctx) {
  int_unable_irq();

  // read  the interrupt identifier so we know the source.
  uint32_t id = GICC0->IAR;

  // handle the interrupt, then clear (or reset) the source.
  if ( id == GIC_SOURCE_TIMER0 ) {

    scheduler( ctx );

    TIMER0->Timer1IntClr = 0x01; // reset timer
  }

  // write the interrupt identifier to signal we're done.
  GICC0->EOIR = id;

  int_enable_irq();

  return;
}


// handle supervisor interrupt calls
void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00: { // 0x00 => yield()
      scheduler( ctx );
      break;
    }
    case 0x01: { // 0x01 => write( fd, x, n )
      hilevel_write( ctx );
      break;
    }
    case 0x03: { // 0x00 => fork()
      hilevel_fork( ctx );
      break;
    }
    case 0x04: { //0x04 => exit()
      hilevel_exit( ctx );
      break;
    }
    case 0x05: { // 0x05 => exec()
      hilevel_exec( ctx );
      break;
    }
    case 0x07: { // 0x05 => pipe_open()
      hilevel_pipe_open( ctx );
      break;
    }
    case 0x08: { // 0x05 => pipe_write()
      hilevel_pipe_write( ctx );
      break;
    }
    case 0x09: { // 0x05 => pipe_read()
      hilevel_pipe_read( ctx );
      break;
    }
    case 0x10: { // 0x05 => pipe_close()
      hilevel_pipe_close( ctx );
      break;
    }
    case 0x11: { // 0x05 => get_proc_id()
      hilevel_get_proc_id( ctx );
      break;
    }
    default: { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
