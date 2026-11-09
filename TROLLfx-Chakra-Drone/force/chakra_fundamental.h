#pragma once
// TROLLfx original pitch controller for Chakra Force instrument.
#include <algorithm>
#include <cmath>
namespace chakra {
enum class RootSource { Chakra, ManualHz, Midi };
struct RootControls {
 RootSource source=RootSource::Chakra;
 int chakra=3;
 float manual_hz=110.f;
 int octave=0;
 int semitone=0;
 float cents=0.f;
 float glide_ms=0.f;
};
class Fundamental {
public:
 explicit Fundamental(float rate=44100.f):rate_(rate>0?rate:44100.f){}
 void set(RootControls v) noexcept {
  v.chakra=std::clamp(v.chakra,0,6);
  v.manual_hz=std::clamp(v.manual_hz,15.f,2000.f);
  v.octave=std::clamp(v.octave,-4,4);
  v.semitone=std::clamp(v.semitone,-12,12);
  v.cents=std::clamp(v.cents,-100.f,100.f);
  v.glide_ms=std::clamp(v.glide_ms,0.f,12000.f);
  controls_=v; updateTarget();
 }
 void midiNote(int note) noexcept {midi_=std::clamp(note,0,127);updateTarget();}
 void trigger() noexcept {current_log_=target_log_;started_=true;}
 float nextHz() noexcept {
  if(!started_){current_log_=target_log_;started_=true;}
  if(controls_.glide_ms<=0.f) current_log_=target_log_;
  else {
   const float a=1.f-std::exp(-1.f/(rate_*controls_.glide_ms*.001f));
   current_log_+=(target_log_-current_log_)*a;
  }
  return std::exp2(current_log_);
 }
 float targetHz()const noexcept{return std::exp2(target_log_);}
 const RootControls& controls()const noexcept{return controls_;}
private:
 void updateTarget()noexcept {
  static constexpr float roots[7]={396,417,528,639,741,852,963};
  float hz=controls_.source==RootSource::Chakra?roots[controls_.chakra]:
    controls_.source==RootSource::ManualHz?controls_.manual_hz:
    440.f*std::exp2((midi_-69.f)/12.f);
  target_log_=std::log2(hz)+(12.f*controls_.octave+controls_.semitone+controls_.cents*.01f)/12.f;
 }
 RootControls controls_{};
 float rate_,current_log_=0.f,target_log_=std::log2(639.f);
 int midi_=69;
 bool started_=false;
};
}
