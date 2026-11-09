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
    ("    float notes_last;", "    chakra::SubVoice sub;\n    float notes_last;"),
    ('    add(P_ENUM, OFF(chakra_gate), 0, 1, "chakra_gate");',
     '''    add(P_ENUM, OFF(chakra_gate), 0, 1, "chakra_gate");
    add(P_FLOAT, OFF(sub_division), 1, 10, "sub_division");
    add(P_FLOAT, OFF(sub_glide_ms), 0, 5000, "sub_glide_ms");
    add(P_FLOAT, OFF(sub_fold), 0, 1, "sub_fold");
    add(P_FLOAT, OFF(sub_mix), 0, 1, "sub_mix");
    add(P_ENUM, OFF(sub_harmonic_lock), 0, 1, "sub_harmonic_lock");
    add(P_ENUM, OFF(sub_phase_lock), 0, 1, "sub_phase_lock");
    add(P_ENUM, OFF(sub_enabled), 0, 1, "sub_enabled");'''),
    ("    inst->engine.set_params(inst->params);\n    set_error(inst, NULL);\n    return inst;",
     """    inst->engine.set_params(inst->params);
    inst->sub.configure(inst->params.sub_division, inst->params.sub_glide_ms,
                        inst->params.sub_fold, inst->params.sub_mix,
                        inst->params.sub_harmonic_lock != 0, inst->params.sub_phase_lock != 0);
    inst->last_sub_gate = inst->params.chakra_gate != 0;\n    if (inst->params.chakra_gate && inst->params.sub_enabled) {
        static const float hz[] = {396, 417, 528, 639, 741, 852, 963};
        const float root_hz = inst->params.chakra_source == 0
            ? hz[inst->params.chakra_root] : inst->params.chakra_hz;
        inst->sub.trigger(root_hz);
    }
    set_error(inst, NULL);
    return inst;"""),
    ("        } else if (cc == 120 || cc == 123) {   // all sound off / all notes off\n            inst->params.chakra_gate = 0;\n            inst->engine.set_params(inst->params);\n            inst->engine.all_notes_off();",
     """            inst->params.chakra_gate = 0;
            inst->engine.set_params(inst->params);
            inst->sub.silence();
            inst->engine.all_notes_off();"""),
    ('    } else if (strcmp(key, "all_notes_off") == 0) {',
     '    } else if (strcmp(key, "all_notes_off") == 0) {'),
    ('    inst->engine.set_params(inst->params);\n}\n\nstatic int get_param(',
     """    const bool sub_gate_rising = inst->params.chakra_gate && !inst->last_sub_gate;\n    const bool sub_gate_falling = !inst->params.chakra_gate && inst->last_sub_gate;\n    inst->last_sub_gate = inst->params.chakra_gate != 0;
    static const float hz[] = {396, 417, 528, 639, 741, 852, 963};
    const float root_hz = inst->params.chakra_source == 0
        ? hz[inst->params.chakra_root] : inst->params.chakra_hz;
    inst->sub.configure(inst->params.sub_division, inst->params.sub_glide_ms,
                        inst->params.sub_fold, inst->params.sub_mix,
                        inst->params.sub_harmonic_lock != 0, inst->params.sub_phase_lock != 0);
    if (!inst->params.sub_enabled) inst->sub.silence();
    else if (inst->params.chakra_gate && !sub_was_active) inst->sub.trigger(root_hz);
    else if (!inst->params.chakra_gate && sub_was_active) inst->sub.release();
    inst->engine.set_params(inst->params);
}

static int get_param("""),
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
params = document["chain_params"]
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
layout = root / "vst/layout.conf"
ui = layout.read_text()
old = 'qlinks "CHAKRA" = chakra_source,chakra_root,chakra_hz,chakra_gate,model,harmonics,timbre,morph,aux_mix'
new = '''frame x=36 y=680 w=1208 h=96 title="SUB BASS ENGINE"
knob cx=190 cy=728 r=27 label="DIVIDE" key=sub_division img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=350 cy=728 r=27 label="SLEW" key=sub_glide_ms img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=510 cy=728 r=27 label="FOLD" key=sub_fold img=images/knob_small.svg base=images/knob_shadow.svg
knob cx=670 cy=728 r=27 label="MIX" key=sub_mix img=images/knob_small.svg base=images/knob_shadow.svg
toggle cx=840 cy=728 label="HARM LOCK" key=sub_harmonic_lock img=images/led_off.svg img_on=images/led_on.svg
toggle cx=995 cy=728 label="PHASE" key=sub_phase_lock img=images/led_off.svg img_on=images/led_on.svg
toggle cx=1140 cy=728 label="SUB ON" key=sub_enabled img=images/led_off.svg img_on=images/led_on.svg
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
