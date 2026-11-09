#!/usr/bin/env python3
"""Add the independent Sub Engine to a checkout already patched by add_chakra_page.py.

Reproducible local source transform, for the separate EXPERIMENTAL build.
Leaves the working Force installation untouched.
"""
from pathlib import Path
import json
import shutil
import sys

if len(sys.argv) != 2:
    raise SystemExit("Usage: python3 apply_sub_engine.py /path/to/patched-mpc-vst-plaits")
root = Path(sys.argv[1]).resolve()
here = Path(__file__).resolve().parent

def modify(relative, edits):
    path = root / relative
    source = path.read_text()
    for before, after in edits:
        occurrences = source.count(before)
        if occurrences != 1:
            raise SystemExit(f"{relative}: expected one occurrence, found {occurrences}: {before[:70]!r}")
        source = source.replace(before, after)
    path.write_text(source)

shutil.copyfile(here / "chakra_sub_engine.h", root / "src/dsp/chakra_sub_engine.h")
shutil.copyfile(here / "test_chakra_sub_host.c", root / "tests/chakra_sub.c")

modify("src/dsp/plaits_move_engine.h", [
    ("    int chakra_gate;\n} ppf_params_t;",
     """    int chakra_gate;
    /* Experimental independent sub voice; new parameters are appended for ABI safety. */
    float sub_division;
    float sub_glide_ms;
    float sub_fold;
    float sub_mix;
    int sub_harmonic_lock;
    int sub_phase_lock;
    int sub_enabled;
} ppf_params_t;"""),
])
modify("src/dsp/plaits_move_engine.cpp", [
    ("    params->chakra_gate = 0;\n",
     """    params->chakra_gate = 0;
    params->sub_division = 10.0f;
    params->sub_glide_ms = 900.0f;
    params->sub_fold = 0.2f;
    params->sub_mix = 0.5f;
    params->sub_harmonic_lock = 1;
    params->sub_phase_lock = 1;
    params->sub_enabled = 1;
"""),
])
modify("src/dsp/freak_plugin.cpp", [
    ('#include "plaits_move_engine.h"', '#include "plaits_move_engine.h"\n#include "chakra_sub_engine.h"'),
    ("    float notes_last;", """    chakra::SubVoice sub;
    bool midi_down[128];
    float midi_velocity[128];
    int midi_order[128];
    int midi_count;
    float notes_last;"""),
    ('    add(P_ENUM, OFF(chakra_gate), 0, 1, "chakra_gate");',
     '''    add(P_ENUM, OFF(chakra_gate), 0, 1, "chakra_gate");
    add(P_FLOAT, OFF(sub_division), 1, 10, "sub_division");
    add(P_FLOAT, OFF(sub_glide_ms), 0, 5000, "sub_glide_ms");
    add(P_FLOAT, OFF(sub_fold), 0, 1, "sub_fold");
    add(P_FLOAT, OFF(sub_mix), 0, 1, "sub_mix");
    add(P_ENUM, OFF(sub_harmonic_lock), 0, 1, "sub_harmonic_lock");
    add(P_ENUM, OFF(sub_phase_lock), 0, 1, "sub_phase_lock");
    add(P_ENUM, OFF(sub_enabled), 0, 1, "sub_enabled");'''),
    ("}\n\nstatic void *create_instance(const char *module_dir, const char *json_defaults) {",
     """}

static float chakra_root_hz(const freak_instance_t *inst) {
    static const float roots[] = {396.0f, 417.0f, 528.0f, 639.0f, 741.0f, 852.0f, 963.0f};
    return inst->params.chakra_source == 0 ? roots[inst->params.chakra_root] : inst->params.chakra_hz;
}

static void midi_remove(freak_instance_t *inst, int note) {
    int out = 0;
    for (int i = 0; i < inst->midi_count; ++i) {
        if (inst->midi_order[i] != note) inst->midi_order[out++] = inst->midi_order[i];
    }
    inst->midi_count = out;
}

static void midi_push(freak_instance_t *inst, int note) {
    midi_remove(inst, note);
    if (inst->midi_count < 128) inst->midi_order[inst->midi_count++] = note;
}

static void sync_sub(freak_instance_t *inst, bool retrigger) {
    inst->sub.configure(inst->params.sub_division, inst->params.sub_glide_ms,
                        inst->params.sub_fold, inst->params.sub_mix,
                        inst->params.sub_harmonic_lock != 0, inst->params.sub_phase_lock != 0);
    if (!inst->params.sub_enabled) {
        inst->sub.silence();
        return;
    }
    float root = 0.0f;
    float velocity = 0.8f;
    if (inst->midi_count > 0) {
        const int note = inst->midi_order[inst->midi_count - 1];
        root = 440.0f * powf(2.0f, ((float)note - 69.0f) / 12.0f);
        velocity = inst->midi_velocity[note];
    } else if (inst->params.chakra_gate) {
        root = chakra_root_hz(inst);
    } else {
        inst->sub.release();
        return;
    }
    if (retrigger || !inst->sub.active()) inst->sub.trigger(root, velocity);
    else inst->sub.retune(root);
}

static void stop_all(freak_instance_t *inst, bool hard) {
    inst->params.chakra_gate = 0;
    inst->sustain = false;
    memset(inst->sustained, 0, sizeof inst->sustained);
    memset(inst->midi_down, 0, sizeof inst->midi_down);
    memset(inst->midi_velocity, 0, sizeof inst->midi_velocity);
    inst->midi_count = 0;
    inst->engine.set_params(inst->params);
    inst->engine.all_notes_off();
    if (hard) inst->sub.silence();
    else inst->sub.release();
}

static void *create_instance(const char *module_dir, const char *json_defaults) {"""),
    ("    inst->sustain = false;\n    memset(inst->sustained, 0, sizeof inst->sustained);\n    inst->notes_last = -1.0f;",
     """    inst->sustain = false;
    memset(inst->sustained, 0, sizeof inst->sustained);
    memset(inst->midi_down, 0, sizeof inst->midi_down);
    memset(inst->midi_velocity, 0, sizeof inst->midi_velocity);
    inst->midi_count = 0;
    inst->notes_last = -1.0f;"""),
    ("    inst->engine.set_params(inst->params);\n    set_error(inst, NULL);\n    return inst;",
     """    inst->engine.set_params(inst->params);
    sync_sub(inst, true);
    set_error(inst, NULL);
    return inst;"""),
    ("        inst->sustained[note] = false;\n        inst->engine.note_on(note, (float)(msg[2] & 0x7F) / 127.0f);",
     """        const float velocity = (float)(msg[2] & 0x7F) / 127.0f;
        inst->sustained[note] = false;
        inst->midi_down[note] = true;
        inst->midi_velocity[note] = velocity;
        midi_push(inst, note);
        inst->engine.note_on(note, velocity);
        sync_sub(inst, true);"""),
    ("        int note = msg[1] & 0x7F;\n        if (inst->sustain) {\n            inst->sustained[note] = true;\n        } else {\n            inst->engine.note_off(note);\n        }",
     """        int note = msg[1] & 0x7F;
        inst->midi_down[note] = false;
        if (inst->sustain) {
            inst->sustained[note] = true;
        } else {
            inst->engine.note_off(note);
            midi_remove(inst, note);
            sync_sub(inst, false);
        }"""),
    ("                        inst->sustained[n] = false;\n                        inst->engine.note_off(n);",
     """                        inst->sustained[n] = false;
                        inst->engine.note_off(n);
                        midi_remove(inst, n);"""),
    ("            inst->sustain = down;",
     """            inst->sustain = down;
            if (!down) sync_sub(inst, false);"""),
    ("        } else if (cc == 120 || cc == 123) {   // all sound off / all notes off\n            inst->params.chakra_gate = 0;\n            inst->engine.set_params(inst->params);\n            inst->engine.all_notes_off();\n            inst->sustain = false;\n            memset(inst->sustained, 0, sizeof(inst->sustained));",
     """        } else if (cc == 120 || cc == 123) {   // all sound off / all notes off
            stop_all(inst, cc == 120);"""),
    ("    set_error(inst, NULL);\n\n    if (strcmp(key, \"state\") == 0) {",
     """    set_error(inst, NULL);
    const bool old_hold = inst->params.chakra_gate != 0;
    const bool old_enabled = inst->params.sub_enabled != 0;

    if (strcmp(key, \"state\") == 0) {"""),
    ("            inst->params.chakra_gate = 0;\n            inst->engine.set_params(inst->params);\n            inst->engine.all_notes_off();",
     """            stop_all(inst, false);"""),
    ('    inst->engine.set_params(inst->params);\n}\n\nstatic int get_param(',
     """    inst->engine.set_params(inst->params);
    const bool retrigger = inst->params.sub_enabled &&
        ((!old_enabled) || (!old_hold && inst->params.chakra_gate && inst->midi_count == 0));
    sync_sub(inst, retrigger);
}

static int get_param("""),
    ('if (strcmp(key, "name") == 0) return snprintf(buf, buf_len, "TROLLfx Chakra Plaits");',
     'if (strcmp(key, "name") == 0) return snprintf(buf, buf_len, "TROLLfx Chakra Plaits SUB");'),
    ("        inst->engine.render(left, right, n);\n",
     """        inst->engine.render(left, right, n);
        for (int i = 0; i < n; ++i) {
            const float sub = inst->sub.process();
            left[i] += sub;
            right[i] += sub;
        }
"""),
])
# Include the new sub controls as append-only chain parameters.
p = root / "src/module.json"
document = json.loads(p.read_text())
if document.get("id") != "trollfx-chakra-plaits":
    raise SystemExit("Expected base Chakra module id")
document["id"] = "trollfx-chakra-plaits-sub"
document["name"] = "TROLLfx Chakra Plaits SUB"
params = document["capabilities"]["chain_params"]
new_params = [
    {"key": "sub_division", "name": "SUB DIVIDE", "type": "float", "min": 1, "max": 10, "default": 10},
    {"key": "sub_glide_ms", "name": "SUB SLEW ms", "type": "float", "min": 0, "max": 5000, "default": 900},
    {"key": "sub_fold", "name": "WAVEFOLD", "type": "float", "min": 0, "max": 1, "default": 0.2},
    {"key": "sub_mix", "name": "SUB MIX", "type": "float", "min": 0, "max": 1, "default": 0.5},
    {"key": "sub_harmonic_lock", "name": "HARMONIC LOCK", "type": "enum", "options": ["Free", "Integer"], "default": 1},
    {"key": "sub_phase_lock", "name": "PHASE RESET", "type": "enum", "options": ["Free", "Reset"], "default": 1},
    {"key": "sub_enabled", "name": "SUB ENGINE", "type": "enum", "options": ["Off", "On"], "default": 1},
]
keys = {p["key"] for p in params}
if any(p["key"] in keys for p in new_params):
    raise SystemExit("Sub parameters already present")
insert = next(i for i,p in enumerate(params) if p["key"] == "model_prev")
params[insert:insert] = new_params
p.write_text(json.dumps(document, indent=2) + "\n")
# Preserve the host's stable VST parameter ordering.
lock = root / "vst/params.lock"
ordered = lock.read_text()
if ordered.count("chakra_gate\n") != 1:
    raise SystemExit("Unexpected Chakra parameter lock file")
ordered = ordered.replace("chakra_gate\n", "chakra_gate\n" +
    "".join(p["key"] + "\n" for p in new_params))
lock.write_text(ordered)
layout = root / "vst/layout.conf"
ui = layout.read_text()
old = '''frame x=36 y=108 w=760 h=238 title="CHAKRA FUNDAMENTAL"
enum_h cx=416 cy=182 sw=250 label="SOURCE" key=chakra_source options="PRESET,MANUAL HZ"
popup cx=240 cy=276 w=340 h=52 label="PRESET" key=chakra_root wheel=1 when=chakra_source:Preset
knob cx=592 cy=276 r=44 label="MANUAL Hz" key=chakra_hz img=images/knob_big.svg base=images/knob_shadow.svg when="chakra_source:Manual Hz"
toggle cx=1030 cy=228 label="HOLD" key=chakra_gate img=images/led_off.svg img_on=images/led_on.svg
frame x=36 y=382 w=1208 h=286 title="NATIVE PLAITS CONTROLS"
popup cx=188 cy=478 w=280 h=48 label="MODEL" key=model wheel=1
knob cx=450 cy=524 r=42 label="HARMONICS" key=harmonics img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=650 cy=524 r=42 label="TIMBRE" key=timbre img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=850 cy=524 r=42 label="MORPH" key=morph img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=1050 cy=524 r=42 label="OUT / AUX" key=aux_mix img=images/knob_big.svg base=images/knob_shadow.svg
qlinks "CHAKRA" = chakra_source,chakra_root,chakra_hz,chakra_gate,model,harmonics,timbre,morph,aux_mix'''
new = '''frame x=36 y=108 w=1208 h=200 title="CHAKRA FUNDAMENTAL"
enum_h cx=300 cy=166 sw=205 label="SOURCE" key=chakra_source options="PRESET,MANUAL HZ"
popup cx=300 cy=246 w=360 h=46 label="PRESET" key=chakra_root wheel=1 when=chakra_source:Preset
knob cx=610 cy=226 r=36 label="MANUAL Hz" key=chakra_hz img=images/knob_big.svg base=images/knob_shadow.svg when="chakra_source:Manual Hz"
toggle cx=1030 cy=206 label="HOLD" key=chakra_gate img=images/led_off.svg img_on=images/led_on.svg
frame x=36 y=322 w=1208 h=152 title="SUB BASS ENGINE"
knob cx=145 cy=402 r=29 label="DIVIDE" key=sub_division img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=305 cy=402 r=29 label="SLEW" key=sub_glide_ms img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=465 cy=402 r=29 label="FOLD" key=sub_fold img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=625 cy=402 r=29 label="MIX" key=sub_mix img=images/knob_small.svg base=images/knob_shadow.svg
toggle cx=810 cy=402 label="HARM LOCK" key=sub_harmonic_lock img=images/led_off.svg img_on=images/led_on.svg
toggle cx=980 cy=402 label="PHASE" key=sub_phase_lock img=images/led_off.svg img_on=images/led_on.svg
toggle cx=1140 cy=402 label="SUB ON" key=sub_enabled img=images/led_off.svg img_on=images/led_on.svg
frame x=36 y=488 w=1208 h=180 title="NATIVE PLAITS CONTROLS"
popup cx=175 cy=564 w=250 h=44 label="MODEL" key=model wheel=1
knob cx=445 cy=580 r=34 label="HARMONICS" key=harmonics img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=650 cy=580 r=34 label="TIMBRE" key=timbre img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=855 cy=580 r=34 label="MORPH" key=morph img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=1060 cy=580 r=34 label="OUT / AUX" key=aux_mix img=images/knob_big.svg base=images/knob_shadow.svg
qlinks "CHAKRA" = chakra_source,chakra_root,chakra_hz,chakra_gate,sub_division,sub_glide_ms,sub_fold,sub_mix,sub_harmonic_lock,sub_phase_lock,sub_enabled,harmonics,timbre,morph,aux_mix'''
if ui.count(old) != 1:
    raise SystemExit("Expected upstream CHAKRA Q-Link line exactly once")
layout.write_text(ui.replace(old,new))
# Produce a distinct test plugin ID, so the working Force release is not overwritten.
vst = root / "vst/vst.json"
v = json.loads(vst.read_text())
if v.get("uid") != "TcPl":
    raise SystemExit("Expected base Chakra UID TcPl")
v["uid"] = "TcSb"
v["name"] = "Chakra Plaits SUB"
v["version"] = 1100
v["so"] = "chakra_plaits_sub.so"
vst.write_text(json.dumps(v, indent=2)+"\n")
print("Sub Engine added; separate experimental VST identity TcSb; original Chakra plugin not replaced")
