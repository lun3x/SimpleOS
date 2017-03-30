#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
    p++; t = -x; n = 1;
  }
  else {
         t = +x; n = 1;
  }

  while( t >= n ) {
    p++; n *= 10;
  }

  *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int fork( int priority ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 to priority
                "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_FORK), "r" (priority)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void exec( const void* x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
              :
              : "I" (SYS_EXEC), "r" (x)
              : "r0" );

  return;
}

int kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

int open_pipe( pid_t pid1, pid_t pid2 ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 to pid1
                "mov r1, %3 \n" // assign r1 to pid2
                "svc %1     \n" // make system call SYS_PIPE
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_PIPE_OPEN), "r" (pid1), "r" (pid2)
              : "r0" );

  return r;
}

void _write_pipe( int id, int x ) {

  asm volatile( "mov r0, %1 \n" // assign r0 =  id
                "mov r1, %2 \n" // assign r1 =   x
                "svc %0     \n" // make system call SYS_PIPE_WRITE
              :
              : "I" (SYS_PIPE_WRITE), "r" (id), "r" (x)
              : "r0", "r1" );

  return;
}

int _read_pipe( int id, int overwrite ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  id
                "mov r1, %3 \n" // assign r1 = overwrite
                "svc %1     \n" // make system call SYS_PIPE_READ
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_PIPE_READ), "r" (id), "r" (overwrite)
              : "r0" );

  return r;
}

void write_pipe( int id, int x ) {
  _write_pipe(id, x);
  int response = x;
  while (response != -1) {
    response = _read_pipe(id, 0); //check value in pipe without overwrite
  }
}


int read_pipe( int id ) {
  int response = -1;
  while (response == -1) {
    response = _read_pipe( id, 1 );
  }
  return response;
}

void close_pipe( int id ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  id
                "svc %0     \n" // make system call SYS_KILL
              :
              : "I" (SYS_PIPE_CLOSE), "r" (id)
              : "r0" );

  return;
}

int get_proc_id() {
  int r;

  asm volatile( "svc %1     \n" // make svc call SYS_ID
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_ID)
              : "r0" );

  return r;
}
