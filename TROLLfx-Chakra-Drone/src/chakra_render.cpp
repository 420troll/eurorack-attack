// TROLLfx Chakra Drone: original proof-of-concept DSP, no external DSP copied.
// Offline WAV prototype; not yet a VST or a Force plugin.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr double pi = 3.14159265358979323846;
constexpr int sampleRate = 48000;
constexpr std::array<double, 7> chakraHz = {396,417,528,639,741,852,963};
constexpr std::array<const char*, 7> chakraNames = {
    "Root","Sacral","Solar Plexus","Heart","Throat","Third Eye","Crown"
};
constexpr std::array<int, 4> chordSemitones = {0, 4, 7, 12};
constexpr size_t tableSize = 2048;

void write16(std::ofstream& out, uint16_t v) {
    char b[2] = {char(v & 255), char((v >> 8) & 255)};
    out.write(b, 2);
}
void write32(std::ofstream& out, uint32_t v) {
    char b[4] = {char(v & 255),char((v >> 8) & 255),
                 char((v >> 16) & 255),char((v >> 24) & 255)};
    out.write(b, 4);
}
void wav(const std::string& path, const std::vector<int16_t>& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot write WAV: " + path);
    const uint32_t bytes = static_cast<uint32_t>(data.size() * sizeof(int16_t));
    out.write("RIFF",4); write32(out,36 + bytes); out.write("WAVE",4);
    out.write("fmt ",4); write32(out,16); write16(out,1);
    write16(out,2); write32(out,sampleRate);
    write32(out,sampleRate * 4); write16(out,4); write16(out,16);
    out.write("data",4); write32(out,bytes);
    for (int16_t s : data) write16(out,static_cast<uint16_t>(s));
}
double wrap(double x) { return x - std::floor(x); }
double clamp(double x) { return std::clamp(x,-1.0,1.0); }
struct Table {
    std::array<double, tableSize> sine{}, harmonic{};
    Table() {
        for(size_t i=0;i<tableSize;++i) {
            const double p=2*pi*double(i)/double(tableSize);
            sine[i]=std::sin(p);
            harmonic[i]=(std::sin(p)+0.36*std::sin(2*p)+
                         0.20*std::sin(3*p)+0.10*std::sin(5*p))/1.66;
        }
    }
    double lookup(const std::array<double,tableSize>& t,double phase) const {
        const double pos=wrap(phase)*tableSize;
        const size_t i=static_cast<size_t>(pos);
        const size_t j=(i+1)%tableSize;
        return t[i]+(t[j]-t[i])*(pos-i);
    }
    double sample(double phase,double morph) const {
        return (1-morph)*lookup(sine,phase)+morph*lookup(harmonic,phase);
    }
};
struct Voice { double phase=0; double frequency=0; double pan=0; };
std::vector<int16_t> render(int chakra,double seconds,double morph,double lfoHz) {
    const size_t frames=static_cast<size_t>(seconds*sampleRate);
    if(frames > 3600ULL*sampleRate) throw std::runtime_error("Duration too long");
    std::vector<int16_t> audio; audio.reserve(frames*2);
    const Table table;
    std::array<Voice,4> voices{};
    for(size_t i=0;i<voices.size();++i) {
        voices[i].frequency=chakraHz[chakra]*std::pow(2.0,chordSemitones[i]/12.0);
        voices[i].pan=static_cast<double>(i)/3.0;
    }
    for(size_t i=0;i<frames;++i) {
        const double t=double(i)/sampleRate;
        const double slow=std::sin(2*pi*lfoHz*t);
        const double position=std::clamp(morph+0.23*slow,0.0,1.0);
        const double fade=std::min({1.0,t/0.08,(seconds-t)/0.35});
        double left=0,right=0;
        for(auto& v:voices) {
            v.phase=wrap(v.phase+v.frequency/sampleRate);
            const double s=table.sample(v.phase,position);
            left += s*(1-v.pan);
            right += s*v.pan;
        }
        const double gain=0.34*std::max(0.0,fade);
        audio.push_back(static_cast<int16_t>(std::lround(clamp(left*gain)*32767)));
        audio.push_back(static_cast<int16_t>(std::lround(clamp(right*gain)*32767)));
    }
    return audio;
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc < 2 || argc > 6) {
            std::cout << "Usage: chakra_render OUTPUT.wav [chakra 1-7] [seconds] [morph 0-1] [LFO Hz]\n";
            return argc < 2 ? 1 : 0;
        }
        const int chakra=argc>2?std::stoi(argv[2]):4;
        const double seconds=argc>3?std::stod(argv[3]):12.0;
        const double morph=argc>4?std::stod(argv[4]):0.5;
        const double lfoHz=argc>5?std::stod(argv[5]):0.15;
        if(chakra<1 || chakra>7 || !std::isfinite(seconds) || seconds<=0 ||
           !std::isfinite(morph) || morph<0 || morph>1 ||
           !std::isfinite(lfoHz) || lfoHz<0 || lfoHz>20)
            throw std::runtime_error("Invalid parameter");
        wav(argv[1],render(chakra-1,seconds,morph,lfoHz));
        std::cout << "Created " << argv[1] << " / " << chakraNames[chakra-1]
                  << " / " << chakraHz[chakra-1] << " Hz base / 48 kHz stereo\n";
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
