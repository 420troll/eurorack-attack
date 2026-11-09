# Manual Fundamental Pitch — Force-first v0.1

This reusable C++17 pitch controller enables a **manual fundamental note** independently of the chakra preset.

Controls:
- Root source: Chakra preset, Manual Hz, or incoming MIDI pitch
- Manual frequency: 15–2000 Hz
- Octave: −4 to +4
- Semitone: −12 to +12
- Fine: ±100 cents
- Glide: 0–12000 ms

When a Chakra preset is selected it supplies the initial fundamental. Switching to Manual makes the pitch independently adjustable. MIDI mode follows note-on pitch, with the octave/semitone/fine offsets retained.

Frequency updates can be swept without releasing the trigger using the `nextHz()` exponential glide. `trigger()` initializes the current pitch. The separate subharmonic sweep can use `nextHz()` as its root frequency.

### Compile the standalone controller test

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I. test_fundamental.cpp -o test_fundamental
./test_fundamental
```

**Integration still required:** connect the controller to the poloq Plaits port's audio-rate pitch path, add parameter keys and knobs in `module.json` and `layout.conf`, and ensure MIDI/Chakra triggers have separate note ownership so note-off does not stop a different held note.

This is a standalone DSP module, **not yet built into the Force plugin**.
