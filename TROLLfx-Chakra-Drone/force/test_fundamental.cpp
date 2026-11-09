#include "chakra_fundamental.h"
#include <cassert>
#include <cmath>
#include <iostream>
static bool near(float a,float b,float eps=.05f){return std::abs(a-b)<eps;}
int main(){
 chakra::Fundamental f;
 assert(near(f.targetHz(),639));
 chakra::RootControls p;
 p.source=chakra::RootSource::ManualHz;p.manual_hz=110;
 f.set(p);assert(near(f.targetHz(),110));
 p.octave=1;f.set(p);assert(near(f.targetHz(),220));
 p.octave=0;p.semitone=12;f.set(p);assert(near(f.targetHz(),220));
 p.semitone=0;p.cents=100;f.set(p);assert(near(f.targetHz(),110*std::exp2(1./12)));
 p.cents=0;p.source=chakra::RootSource::Midi;f.set(p);f.midiNote(60);
 assert(near(f.targetHz(),261.62556f));
 p.source=chakra::RootSource::ManualHz;p.manual_hz=110;p.glide_ms=100;
 f.set(p);f.trigger();
 p.manual_hz=220;f.set(p);
 float prev=f.nextHz();
 for(int i=0;i<10000;++i){float now=f.nextHz();assert(now>=prev-1e-3f);prev=now;}
 assert(near(prev,220));
 std::cout<<"Fundamental controls PASS\n";
}
