#include "wrapping_integers.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "wrap( {}, {} ) called", n, zero_point.raw_value_ );
  return Wrap32 {static_cast<uint32_t>((n + zero_point.raw_value_) & mod32)};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // debug( "unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  uint32_t del = raw_value_ - zero_point.raw_value_;
  uint64_t lower = (checkpoint / max32) * max32;
  if((checkpoint > 0) && ((checkpoint & mod32) == 0)) {
    lower -= max32;
  }
  lower += del;
  uint64_t upper = (checkpoint + max32 - 1) / max32 * max32 + del;
  if(lower > checkpoint) {
    upper = lower, lower -= max32;
    if(upper < lower) swap(upper, lower);
  }
  if(checkpoint <= lower) {
    return lower;
  }
  return checkpoint - lower < upper - checkpoint ? lower : upper;
}