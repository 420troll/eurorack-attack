#!/usr/bin/env python3
"""Apply CHORDS-only Chakra UI and separate SUB page after apply_sub_engine.py.

Run on a disposable upstream checkout after add_chakra_page.py and
apply_sub_engine.py. Never apply to an installed Force plugin folder.
"""
from pathlib import Path
import json
import sys

if len(sys.argv) != 2:
    raise SystemExit("Usage: apply_chakra_chords_v2.py /path/to/patched-mpc-vst-plaits")
root = Path(sys.argv[1]).resolve()

# Fix the actual DSP model, not only the visible UI. Chords is Plaits model 14.
engine = root / "src/dsp/plaits_move_engine.cpp"
source = engine.read_text()
before = "    p.model = clampi(p.model, 0, kMaxEngines - 1);"
after = "    p.model = 14;  // CHORDS-only Chakra V2: ignore host/preset model changes"
if source.count(before) != 1:
    raise SystemExit("Unexpected Plaits engine model setter; aborting")
engine.write_text(source.replace(before, after))

# Force exposed parameter/state to agree with the engine, including saved presets,
# automation and restored JSON. Keep the MODEL parameter index for compatibility.
plugin = root / "src/dsp/freak_plugin.cpp"
plugin_source = plugin.read_text()
anchors = [
    ("    inst->engine.init();\n    inst->engine.set_params(inst->params);",
     "    inst->engine.init();\n    inst->params.model = 14;\n    inst->engine.set_params(inst->params);"),
    ("    inst->engine.set_params(inst->params);\n    const bool retrigger = inst->params.sub_enabled &&",
     "    inst->params.model = 14;\n    inst->engine.set_params(inst->params);\n    const bool retrigger = inst->params.sub_enabled &&"),
]
for old_anchor, new_anchor in anchors:
    if plugin_source.count(old_anchor) != 1:
        raise SystemExit("Unexpected V2 host plugin model anchor")
    plugin_source = plugin_source.replace(old_anchor, new_anchor)
plugin.write_text(plugin_source)

# The original seven SUB parameters and their VST indices stay untouched.
# This new identity preserves both the original TcPl and experimental TcSb.
vst_path = root / "vst/vst.json"
vst = json.loads(vst_path.read_text())
if vst.get("uid") != "TcSb":
    raise SystemExit("Expected SUB V1 base TcSb")
vst.update(uid="TcC2", name="Chakra Chords SUB V2",
           so="chakra_chords_sub_v2.so", version=1200)
vst_path.write_text(json.dumps(vst, indent=2) + "\n")

module_path = root / "src/module.json"
module = json.loads(module_path.read_text())
if module.get("id") != "trollfx-chakra-plaits-sub":
    raise SystemExit("Expected Chakra SUB module")
module["id"] = "trollfx-chakra-chords-sub-v2"
module["name"] = "TROLLfx Chakra Chords SUB V2"
for parameter in module["capabilities"]["chain_params"]:
    if parameter["key"] == "model":
        parameter["default"] = 14
        break
else:
    raise SystemExit("Missing model parameter")
module_path.write_text(json.dumps(module, indent=2) + "\n")

# Tabs and Q-Link groups are distinct. No MODEL selector is exposed.
layout_path = root / "vst/layout.conf"
layout = layout_path.read_text()
if layout.count("[tab CHAKRA]") != 1:
    raise SystemExit("Expected one CHAKRA tab")
prefix, old = layout.split("[tab CHAKRA]", 1)
# Remove the legacy Plaits model browser (LED row, previous/next and popup)
# from the PLAITS tab as well, not only the new CHAKRA tab.
old_prefix = prefix
prefix = "\n".join(line for line in prefix.split("\n")
                   if not (line.lstrip().startswith(("picture ", "button ", "popup "))
                           and any("key=" + key in line for key in
                                   ("model ", "model_prev", "model_next"))))
prefix = prefix.replace(
    'qlinks "PLAITS" = pitch,harmonics,timbre,morph,model,timbre_att,fm_att,morph_att,aux_mix',
    'qlinks "PLAITS" = pitch,harmonics,timbre,morph,-,timbre_att,fm_att,morph_att,aux_mix')
if prefix == old_prefix or 'key=model ' in prefix:
    raise SystemExit("Failed to hide PLAITS model controls")
if "[tab " in old:
    raise SystemExit("Unexpected tabs after CHAKRA; inspect layout before editing")
if 'qlinks "CHAKRA" =' not in old or "key=sub_mix" not in old:
    raise SystemExit("Expected SUB V1 layout; aborting")
new = """[tab CHAKRA]
frame x=36 y=108 w=1208 h=220 title="CHAKRA FREQUENCY"
enum_h cx=310 cy=170 sw=205 label="SOURCE" key=chakra_source options="PRESET,MANUAL HZ"
popup cx=310 cy=265 w=360 h=46 label="PRESET" key=chakra_root wheel=1 when=chakra_source:Preset
knob cx=650 cy=250 r=40 label="MANUAL Hz" key=chakra_hz img=images/knob_big.svg base=images/knob_shadow.svg when="chakra_source:Manual Hz"
toggle cx=1030 cy=245 label="HOLD" key=chakra_gate img=images/led_off.svg img_on=images/led_on.svg
frame x=36 y=350 w=1208 h=318 title="PLAITS CHORDS ONLY"
knob cx=260 cy=515 r=44 label="HARMONICS" key=harmonics img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=525 cy=515 r=44 label="TIMBRE" key=timbre img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=790 cy=515 r=44 label="MORPH" key=morph img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=1050 cy=515 r=44 label="OUT / AUX" key=aux_mix img=images/knob_big.svg base=images/knob_shadow.svg
qlinks "CHAKRA" = chakra_source,chakra_root,chakra_hz,chakra_gate,harmonics,timbre,morph,aux_mix

[tab SUB]
frame x=36 y=108 w=1208 h=260 title="SUB OSCILLATOR"
knob cx=245 cy=248 r=48 label="DIVIDE" key=sub_division img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=645 cy=248 r=48 label="PITCH SLEW" key=sub_glide_ms img=images/knob_big.svg base=images/knob_shadow.svg
toggle cx=1050 cy=235 label="SUB ON" key=sub_enabled img=images/led_off.svg img_on=images/led_on.svg
frame x=36 y=390 w=1208 h=278 title="WAVEFOLD AND HARMONIC LOCK"
knob cx=255 cy=522 r=46 label="WAVEFOLD" key=sub_fold img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=575 cy=522 r=46 label="SUB MIX" key=sub_mix img=images/knob_big.svg base=images/knob_shadow.svg
toggle cx=865 cy=520 label="HARM LOCK" key=sub_harmonic_lock img=images/led_off.svg img_on=images/led_on.svg
toggle cx=1070 cy=520 label="PHASE RESET" key=sub_phase_lock img=images/led_off.svg img_on=images/led_on.svg
qlinks "SUB" = sub_enabled,sub_division,sub_glide_ms,sub_fold,sub_mix,sub_harmonic_lock,sub_phase_lock
"""
layout_path.write_text(prefix + new)
assert "key=model " not in new
assert "sub_mix" not in new.split("[tab SUB]")[0]
assert "timbre" not in new.split("[tab SUB]")[1]
# The SUB V1 host test is copied by apply_sub_engine.py; update its identity
# and prove attempts to select another model always return CHORDS (index 14).
test_path = root / "tests/chakra_sub.c"
test = test_path.read_text()
old_uid = 'CHECK(a&&a->uid==0x54635362,"UID is TcSb");'
new_uid = 'CHECK(a&&a->uid==0x54634332,"UID is TcC2");'
if test.count(old_uid) != 1:
    raise SystemExit("Unexpected SUB host UID assertion")
test = test.replace(old_uid, new_uid)
old_model = 'CHECK(isfinite(r)&&r>1e-6,"model %d renders",m);'
new_model = ('CHECK(isfinite(r)&&r>1e-6&&'
             'fabs(a->getP(a,idx("model"))-(14.0f/23.0f))<1e-5,'
             '"model %d resolves to CHORDS",m);')
if test.count(old_model) != 1:
    raise SystemExit("Unexpected SUB host model test")
test = test.replace(old_model, new_model)
test_path.write_text(test)
print("CHORDS-only DSP and independent CHAKRA/SUB UI applied: TcC2")
