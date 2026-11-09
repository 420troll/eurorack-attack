#include "chakra_pitch.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
  using namespace chakra;
  assert(rootHz(3)==639.0);
  assert(std::abs(noteFromHz(440.0)-69.0)<1e-12);
  for(double f:kRootsHz) assert(std::abs(hzFromNote(noteFromHz(f))-f)<1e-10);
  auto c=chordHz(528.0,Chord::Major);
  assert(std::abs(c[0]-528.0)<1e-9);
  assert(std::abs(c[3]-1056.0)<1e-9);
  auto s=subHz(528.0);
  assert(s[0]==264.0 && s[1]==176.0 && s[2]==132.0 && s[3]==66.0);
  bool rejected=false;try{noteFromHz(-3);}catch(const std::invalid_argument&){rejected=true;}
  assert(rejected);
  std::cout<<"Chakra pitch/chord/subharmonic unit tests PASS\n";
}
