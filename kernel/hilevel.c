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
pcb_t pcb[ MAX_PROGS ], *current = NULL;

pipe_t pipes[ MAX_PIPES ];

// current maximum allocated program id
pid_t max_pid = 0;

// entry points for programs
extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_console();

// location of top of stack of console
extern uint32_t tos_user_progs;


// find pcb index from pid
int find_pcb_index(pid_t pid) {
  for (int i = 0; i < MAX_PROGS; i++) {
    if (pcb[ i ].pid == pid) {
      return i;
    }
  }

  return -1; // return error if program not found (?)
}


// return the next program to be executed in the pcb according to round robin
int find_next_pcb_index_rr() {
  int current_index = find_pcb_index(current->pid);

  for (int i = 1; i < MAX_PROGS; i++) {
    int next_index = (current_index + i) % MAX_PROGS;
    if (pcb[ next_index ].status == EXECUTING) {
      return next_index;
    }
  }
  return current_index;
}


// return the next program to be executed in the pcb according to a priority queue
int find_next_pcb_index_pq() {
  int max_priority = 0, index = 0;

  for (int i = 0; i < MAX_PROGS; i++) {
    if (pcb[i].status == EXECUTING && pcb[i].priority >= max_priority) {
      max_priority = pcb[i].priority;
      index = i;
    }
  }

  return index;
}


// return the next free index int the pcb
int find_free_pcb_index() {
  for (int i = 0; i < MAX_PROGS; i++) {
    if (pcb[i].status == TERMINATED) {
      return i; // return index of free pcb
    }
  }

  return -1; // return error if no free pcbs
}


// increase priority of all processes not currently executing
void age_processes(int current_pcb_index) {
  for (int i = 0; i < MAX_PROGS; i++) {
    if (i != current_pcb_index && pcb[i].status == EXECUTING) {
      pcb[i].priority++;
    }
  }
}


// scheduler, round-robin approach
void scheduler(ctx_t* ctx) {
  int current_pcb_index = find_pcb_index(current->pid);  // find current program index
  int next_pcb_index = find_next_pcb_index_pq();            // find index of next program to be executed (if it exists)

  age_processes(current_pcb_index);

  if (next_pcb_index != current_pcb_index) {                        // if there is another program to execute
    memcpy( &pcb[ current_pcb_index ].ctx, ctx, sizeof( ctx_t ) );  // save current program to its place in pcb table

    memcpy( ctx, &pcb[ next_pcb_index ].ctx, sizeof( ctx_t ) ); // copy the context in the index of the new program into context passed in
    current = &pcb[ next_pcb_index ];                           // point the current pointer to the new program to be executed
  }

  return;
}


// create new child process identical to parent
void hilevel_fork( ctx_t *ctx ) {
  int free_pcb_index = find_free_pcb_index(); // find where to put new program

  memcpy( &pcb[ free_pcb_index ].ctx, ctx, sizeof( ctx_t ) ); // copy current ctx to new pcb ctx to make exact copy

  pcb[ free_pcb_index ].pid = max_pid;          // update new process in pcb table with max pid
  pcb[ free_pcb_index ].priority = ctx->gpr[0]; // update new process with correct priority
  max_pid++;                                    // increment max_pid

  int new_tos     = (int) &tos_user_progs - free_pcb_index               * STACK_SIZE; // find tos for new program
  int current_tos = (int) &tos_user_progs - find_pcb_index(current->pid) * STACK_SIZE; // find tos for current program

  int sp_location = current_tos - ctx->sp; // find where in the stack the stack pointer is (dist from current tos)

  pcb[ free_pcb_index ].ctx.sp = new_tos - sp_location; // update the new stack pointer to its correct location in the new stack

  memcpy( (void *) new_tos - STACK_SIZE, (void *) current_tos - STACK_SIZE, STACK_SIZE ); // copy across the current stack into the new stack

  pcb[ free_pcb_index ].ctx.gpr[ 0 ] = 0; // return 0 to child process

  ctx->gpr[ 0 ] = pcb[ free_pcb_index ].pid; // return pid of child process to parent process

  pcb[ free_pcb_index ].status = EXECUTING; // set status of new process to EXECUTING

  return;
}


// load new program image to be executed
void hilevel_exec( ctx_t* ctx ) {
  int current_tos = (int) &tos_user_progs - find_pcb_index(current->pid) * STACK_SIZE; // get current top of stack

  memset( (void *) current_tos - STACK_SIZE, 0, STACK_SIZE); // initialise stack to zeros for security

  ctx->sp = current_tos;   // initialise stack pointer to start of stack
  ctx->pc = ctx->gpr[ 0 ]; // set pc to entry point of new function

  for (int i = 0; i < 13; i++) { // set gprs to 0 for security
    ctx->gpr[ i ] = 0;
  }

  return;
}


// find program in pcb list and remove it
void hilevel_exit( ctx_t* ctx ) {
  pcb[ find_pcb_index(current->pid) ].status = TERMINATED; // set the status to terminated
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
void hilevel_pipe_open( ctx_t *ctx ) {
  int free_pipe_index = find_free_pipe_index();

  if (free_pipe_index >= 0) { // if space for free pipes
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
    if (pipes[ pipe_id ].proc1 == current->pid || pipes[ pipe_id ].proc2 == current->pid) {
      pipes[ pipe_id ].value = data;
    }
  }

  return;
}


//read data from pipe
void hilevel_pipe_read( ctx_t *ctx ) {
  int pipe_id = ctx->gpr[0];

  if (pipes[ pipe_id ].status == OPEN) {
    if (pipes[ pipe_id ].proc1 == current->pid || pipes[ pipe_id ].proc2 == current->pid) {

      ctx->gpr[0] = pipes[ pipe_id ].value;                  // read value into register for return to function
      if (ctx->gpr[1]) pipes[ pipe_id ].value = -1;          // if overwrite flag on, reset pipe value to prevent multiple reads
    }
  }

  return;
}


// close existing pipe
void hilevel_pipe_close( ctx_t *ctx ) {
  int pipe_id = ctx->gpr[0];

  if (pipes[ pipe_id ].status == OPEN) {
    if (pipes[ pipe_id ].proc1 == current->pid || pipes[ pipe_id ].proc2 == current->pid) {
      pipes[ pipe_id ].value = -1;
      pipes[ pipe_id ].status = CLOSED;
    }
  }
}


// get pid of current process
void hilevel_get_proc_id( ctx_t *ctx ) {
  ctx->gpr[0] = current->pid;
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

  TIMER0->Timer1Load  = 0x00001000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack.
   */

  max_pid = 0; // initialise max pid to 0

  // initialise pcb to zeros
  for (int i = 0; i < MAX_PROGS; i++) {
    memset( &pcb[i], 0, sizeof( pcb_t ) );
    pcb[i].status = TERMINATED;
    pcb[i].pid = -1;
  }

  // initialise pipes to 0
  for (int i = 0; i < MAX_PIPES; i++) {
    memset( &pipes[i], 0, sizeof( pipe_t ) );
    pipes[i].status = CLOSED;
    pipes[i].id = -1;
  }

  // intialise first entry to pcb table as console
  pcb[ 0 ].pid      = max_pid;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_user_progs  );
  pcb[ 0 ].priority = 10;
  pcb[ 0 ].status   = EXECUTING;
  max_pid++;

  // once pcb of console initialised, select it to be currently runnning prog
  current = &pcb[ 0 ];
  memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

  // enable irq interrupts
  int_enable_irq();

  return;
}


// handle irq interrupt calls
void hilevel_handler_irq( ctx_t* ctx ) {
  // step 1: disable irq interrupts
  int_unable_irq();

  // step 2: read  the interrupt identifier so we know the source.
  uint32_t id = GICC0->IAR;

  // step 3: handle the interrupt, then clear (or reset) the source.
  if ( id == GIC_SOURCE_TIMER0 ) {

    scheduler( ctx );

    TIMER0->Timer1IntClr = 0x01; // reset timer
  }

  // step 4: write the interrupt identifier to signal we're done.
  GICC0->EOIR = id;

  // step 5: enable irq interrupts
  int_enable_irq();

  return;
}


// handle supervisor interrupt calls
void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
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
