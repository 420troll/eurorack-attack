// Native desktop reference using ORIGINAL Mutable Instruments Plaits Voice DSP.
// This file is TROLLfx glue; upstream source is cloned separately at build time.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "plaits/dsp/voice.h"

namespace {
constexpr int kSampleRate = 48000;
constexpr int kFrames = 12; // Original Plaits engine processing block
constexpr size_t kScratchBytes = 16384;
constexpr std::array<double,7> kChakras = {396,417,528,639,741,852,963};
static_assert(sizeof(plaits::Voice::Frame)==4,"Plaits frame must contain 2 x int16");

void write16(std::ofstream& file, uint16_t x) {
  const char bytes[] = {static_cast<char>(x&255),static_cast<char>((x>>8)&255)};
  file.write(bytes,2);
}
void write32(std::ofstream& file, uint32_t x) {
  const char bytes[] = {static_cast<char>(x&255),static_cast<char>((x>>8)&255),
                        static_cast<char>((x>>16)&255),static_cast<char>((x>>24)&255)};
  file.write(bytes,4);
}
float noteFromHz(double hz) {
  return static_cast<float>(69.0 + 12.0 * std::log2(hz / 440.0));
}
int selectedModel(const std::string& name) {
  if(name=="additive")return 12; // verified upstream Voice::Init registration order
  if(name=="wavetable")return 13;
  if(name=="chord")return 14;
  throw std::runtime_error("Model must be additive, wavetable or chord");
}
int mainRun(int argc,char** argv) {
  if(argc!=5) {
    std::cerr<<"Usage: chakra_native_render OUT.wav FREQ_HZ {additive|wavetable|chord} SECONDS\n";
    return 2;
  }
  const std::string path=argv[1],name=argv[3];
  const double hz=std::stod(argv[2]);
  const double seconds=std::stod(argv[4]);
  if(!(hz>=30 && hz<=5000 && std::isfinite(hz)) || !(seconds>0 && seconds<=60 && std::isfinite(seconds)))
    throw std::runtime_error("Frequency must be 30..5000 Hz; duration 0..60 seconds");
  const int engine=selectedModel(name);
  // Buffer allocation happens at init only, never in the audio render loop.
  std::array<uint8_t,kScratchBytes> memory{};
  stmlib::BufferAllocator allocator(memory.data(),memory.size());
  plaits::Voice voice;
  voice.Init(&allocator);
  plaits::Patch patch{};
  patch.note=noteFromHz(hz);
  patch.engine=engine;
  patch.harmonics=.5f; patch.timbre=.5f; patch.morph=.5f;
  patch.decay=.5f; patch.lpg_colour=.5f;
  plaits::Modulations modulation{};
  modulation.trigger_patched=false; // continuous drone, not a one-shot envelope
  modulation.level_patched=false;
  std::ofstream out(path,std::ios::binary);
  if(!out)throw std::runtime_error("Cannot open output WAV");
  const uint32_t frames=static_cast<uint32_t>(std::ceil(seconds*kSampleRate/kFrames))*kFrames;
  const uint32_t bytes=frames*4;
  out.write("RIFF",4); write32(out,36+bytes); out.write("WAVE",4);
  out.write("fmt ",4);write32(out,16);write16(out,1);write16(out,2);
  write32(out,kSampleRate);write32(out,kSampleRate*4);write16(out,4);write16(out,16);
  out.write("data",4);write32(out,bytes);
  std::array<plaits::Voice::Frame,kFrames> block{};
  int peak=0;
  for(uint32_t n=0;n<frames;n+=kFrames) {
    voice.Render(patch,modulation,block.data(),block.size());
    for(const auto& sample:block) {
      // Preserve original OUT on left and AUX on right (no fake stereo).
      peak=std::max({peak,std::abs(int(sample.out)),std::abs(int(sample.aux))});
      write16(out,static_cast<uint16_t>(sample.out));
      write16(out,static_cast<uint16_t>(sample.aux));
    }
  }
  if(!out)throw std::runtime_error("WAV write failed");
  std::cout<<"Native Plaits "<<name<<" | "<<hz<<" Hz | "<<seconds<<"s | peak "<<peak
           <<" | OUT=L AUX=R | "<<path<<"\n";
  if(peak==0)throw std::runtime_error("Output was silent");
  return 0;
}
} // namespace
int main(int argc,char** argv) {
  try{return mainRun(argc,argv);}
  catch(const std::exception& e){std::cerr<<"Error: "<<e.what()<<"\n";return 1;}
}
