#include "chakra_sub_sweep.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 chakra::SubSweep s(44100);
 chakra::SweepSettings cfg;cfg.duration_ms=1000;cfg.divisor=8;cfg.velocity_shape=1;
 s.setSettings(cfg);s.trigger(528,1);
 auto first=s.process();
 assert(first.plaits_gain==1 && first.progress==0 && std::abs(first.hz-528)<.01f);
 for(int i=1;i<=44100;++i)s.process();
 auto last=s.process();
 assert(std::abs(last.hz-66.f)<.1f && std::abs(last.plaits_gain)<1e-4f);
 s.trigger(639,.25f);
 for(int i=0;i<=44100;++i)s.process();
 assert(std::abs(s.process().plaits_gain-.75f)<.01f);
 s.stop();assert(!s.active()&&s.process().sub==0.f);
 std::cout<<"Chakra sub sweep tests PASS\\n";
}
