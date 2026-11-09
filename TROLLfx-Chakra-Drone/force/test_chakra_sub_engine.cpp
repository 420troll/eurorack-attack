#include "chakra_sub_engine.h"
#include <cassert>
#include <cmath>
#include <iostream>

static bool near(float a,float b,float tolerance){return std::fabs(a-b)<tolerance;}
int main(){
    chakra::SubVoice s(44100.f);
    s.configure(10.f,900.f,.2f,.5f);
    s.trigger(528.f);
    assert(near(s.frequency(),528.f,.01f));
    assert(near(s.target(),52.8f,.01f));
    float first=s.process(); (void)first;
    for(int i=0;i<44100;++i) {float sample=s.process(); assert(std::isfinite(sample));assert(std::fabs(sample)<=1.01f);}
    assert(near(s.frequency(),52.8f,.01f));
    s.configure(7.35f,300.f,.5f,.5f,true,true);
    assert(near(s.target(),528.f/7.f,.01f));
    s.configure(7.35f,300.f,.5f,.5f,false,false);
    assert(near(s.target(),528.f/7.35f,.01f));
    for(int i=0;i<15000;++i) s.process();
    assert(near(s.frequency(),528.f/7.35f,.01f));
    s.release();
    for(int i=0;i<5000;++i) s.process();
    assert(!s.active());
    assert(s.process()==0.f);
    chakra::SubVoice b; b.configure(1.f,0.f,0.f,.5f); b.trigger(396.f);
    b.process(); assert(near(b.frequency(),396.f,.01f));
    b.silence(); assert(b.process()==0.f);
    chakra::SubVoice muted; muted.configure(10.f,900.f,0.f,0.f); muted.trigger(528.f);
    for(int i=0;i<1000;++i) assert(muted.process()==0.f);
    std::cout<<"Chakra Sub Engine PASS\n";
}
