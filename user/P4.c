#include "P4.h"

uint32_t gcd( uint32_t x, uint32_t y ) {
  if     ( x == y ) {
    return x;
  }
  else if( x >  y ) {
    return gcd( x - y, y );
  }
  else if( x <  y ) {
    return gcd( x, y - x );
  }
}

// void main_P4() {
//   while( 1 ) {
//     write( STDOUT_FILENO, "P4", 2 );
//
//     uint32_t lo = 1 <<  4;
//     uint32_t hi = 1 <<  8;
//
//     for( uint32_t x = lo; x < hi; x++ ) {
//       for( uint32_t y = lo; y < hi; y++ ) {
//         uint32_t r = gcd( x, y );
//       }
//     }
//   }
//
//   exit( EXIT_SUCCESS );
// }

void main_P4() {
  int received = read_pipe(0);

  char rec_str[2];
  itoa(rec_str, received);

  write( STDOUT_FILENO, rec_str, 2 );
  exit( EXIT_SUCCESS );
}
