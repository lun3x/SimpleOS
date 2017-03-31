#include "P3.h"

int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

// void main_P3() {
//   for( int i = 0; i < 50; i++ ) {
//     write( STDOUT_FILENO, "P3", 2 );
//
//     uint32_t lo = 1 <<  8;
//     uint32_t hi = 1 << 16;
//
//     for( uint32_t x = lo; x < hi; x++ ) {
//       int r = is_prime( x );
//     }
//   }
//
//   write( STDOUT_FILENO, "endP3", 5 );
//
//   exit( EXIT_SUCCESS );
// }

void main_P3() {
  int pipe_id = open_pipe(2, 3);
  write_pipe(pipe_id, 6);

  write( STDOUT_FILENO, "6", 1 );
  exit( EXIT_SUCCESS );
}
