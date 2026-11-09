#pragma once
// TROLLfx original DSP: sample-by-sample subharmonic sweep at 44.1 kHz.
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace chakra {
struct SweepSettings {
 float duration_ms=900.f;
 float depth=1.f;
 float velocity_shape=.8f;
 int divisor=8;
 bool pitch_glide=true;
};
class SubSweep {
public:
 explicit SubSweep(float rate=44100.f):rate_(rate){}
 void setSettings(SweepSettings s) noexcept {
  s.duration_ms=std::clamp(s.duration_ms,10.f,12000.f);
  s.depth=std::clamp(s.depth,0.f,1.f);
  s.velocity_shape=std::clamp(s.velocity_shape,0.f,1.f);
  if(s.divisor!=2&&s.divisor!=3&&s.divisor!=4&&s.divisor!=8)s.divisor=8;
  cfg_=s;
 }
 void trigger(float hz,float velocity) noexcept {
  if(!std::isfinite(hz)||hz<=0.f)return;
  root_=hz;
  const float v=std::clamp(velocity,0.f,1.f);
  depth_=cfg_.depth*((1.f-cfg_.velocity_shape)+cfg_.velocity_shape*v);
  sample_=0;phase_=0.f;active_=true;
 }
 void stop() noexcept {active_=false;}
 bool active()const noexcept{return active_;}
 float targetHz()const noexcept{return root_/float(cfg_.divisor);}
 struct Frame{float sub;float plaits_gain;float progress;float hz;};
 Frame process() noexcept {
  if(!active_)return {0.f,1.f,0.f,0.f};
  const float total=std::max(1.f,cfg_.duration_ms*.001f*rate_);
  const float t=std::clamp(float(sample_)/total,0.f,1.f);
  const float x=t*t*(3.f-2.f*t);
  const float hz=cfg_.pitch_glide?root_*std::pow(1.f/cfg_.divisor,x):targetHz();
  const float g=depth_*x;
  const float sub=std::sin(phase_*6.2831853071795864769f)*g;
  phase_+=hz/rate_;
  if(phase_>=1.f)phase_-=std::floor(phase_);
  if(sample_<static_cast<uint64_t>(total))++sample_;
  return {sub,1.f-g,t,hz};
 }
private:
 SweepSettings cfg_{};
 float rate_,root_=0.f,phase_=0.f,depth_=1.f;
 uint64_t sample_=0;
 bool active_=false;
};
}
