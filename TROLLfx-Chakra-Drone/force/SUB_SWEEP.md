# Chakra Subharmonic Morph Sweep v0.1

Tested original C++ DSP module for the future Akai Force Plaits wrapper.

- Trigger from Force MIDI note-on or Chakra page trigger: use the true note frequency as `root_hz`, and normalized MIDI velocity as `velocity`.
- Subharmonic target selectable `root/2`, `root/3`, `root/4` or `root/8`.
- Smoothstep pitch movement from root to subharmonic; optional immediate sub pitch with level sweep.
- Crossfade amount follows trigger velocity; 0% to 100% depth adjustable.
- Slider target ranges: sweep 10–12000 ms, depth 0–100%, velocity response 0–100%.
- Original Plaits audio remains the primary signal and is mixed at `frame.plaits_gain`, plus a dedicated sine sub oscillator at `frame.sub`. Apply output gain/headroom in host wrapper.
- No allocations, locks or file access inside `process()`. Render can be called sample-by-sample at 44100 Hz.
- Note-off calls `stop()` in the simplest trigger mode; future release envelopes should fade smoothly.

## Test
```bash
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I. test_sub_sweep.cpp -o sweep_test
./sweep_test
```

## Integration

Include `chakra_sub_sweep.h` into poloq's `freak_plugin.cpp` audio instance and call `trigger(hz,velocity)` from note-on / Chakra tap. During `render_block`, call `process()` for each frame and mix `plaits_l*plaits_gain + sub*sub_level` into left and right channels before float-to-int16.

Important: poloq's original host rate is 44100 Hz, not 48000. The sub oscillator is **new TROLLfx DSP**, not a native Plaits feature; the Plaits algorithm remains unchanged.

## Status

C++ header and offline unit test compiled and passed on a local Linux compiler. Not yet integrated into the Force plugin or hardware tested.
