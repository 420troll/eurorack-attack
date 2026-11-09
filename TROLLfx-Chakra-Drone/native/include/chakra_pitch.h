#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace chakra {
constexpr std::array<double,7> kRootsHz{{396.0,417.0,528.0,639.0,741.0,852.0,963.0}};
enum class Chord : uint8_t { Major, Minor, Sus2, Sus4, Fifths, Unison };
inline double rootHz(unsigned index) {
  if(index>=kRootsHz.size()) throw std::out_of_range("Chakra index must be 0..6");
  return kRootsHz[index];
}
inline double noteFromHz(double hz) {
  if(!std::isfinite(hz) || hz<=0.0) throw std::invalid_argument("Hz must be positive finite");
  return 69.0 + 12.0*std::log2(hz/440.0);
}
inline double hzFromNote(double note) { return 440.0*std::exp2((note-69.0)/12.0); }
inline std::array<double,4> chordHz(double root, Chord mode) {
  if(!std::isfinite(root) || root<=0.0) throw std::invalid_argument("Invalid root");
  std::array<int,4> semitones{};
  switch(mode) {
    case Chord::Major: semitones={{0,4,7,12}}; break;
    case Chord::Minor: semitones={{0,3,7,12}}; break;
    case Chord::Sus2: semitones={{0,2,7,12}}; break;
    case Chord::Sus4: semitones={{0,5,7,12}}; break;
    case Chord::Fifths: semitones={{0,7,12,19}}; break;
    case Chord::Unison: semitones={{0,0,12,12}}; break;
  }
  std::array<double,4> notes{};
  for(size_t i=0;i<4;i++) notes[i]=root*std::exp2(semitones[i]/12.0);
  return notes;
}
inline std::array<double,4> subHz(double root) {
  if(!std::isfinite(root) || root<=0.0) throw std::invalid_argument("Invalid root");
  return {{root/2.0,root/3.0,root/4.0,root/8.0}};
}
} // namespace chakra
