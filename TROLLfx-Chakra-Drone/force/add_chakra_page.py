#!/usr/bin/env python3
"""Patch a fresh checkout of poloq-instruments/mpc-vst-plaits.
Usage: python3 add_chakra_page.py /path/to/mpc-vst-plaits
Does not build or install onto a Force.
"""
import json
import pathlib
import sys

if len(sys.argv) != 2:
    raise SystemExit("Usage: add_chakra_page.py /path/to/mpc-vst-plaits")
root = pathlib.Path(sys.argv[1])
fp = root / "src/dsp/freak_plugin.cpp"
jp = root / "src/module.json"
lp = root / "vst/layout.conf"
vp = root / "vst/vst.json"
for p in (fp,jp,lp,vp):
    if not p.is_file(): raise SystemExit("Missing "+str(p))
s=fp.read_text()
j=json.loads(jp.read_text())
l=lp.read_text()
v=json.loads(vp.read_text())
if "TROLLfx chakra v0.1" in s or "[tab CHAKRA]" in l:
    raise SystemExit("Already patched")
def inject(hay,needle,replace):
    if hay.count(needle)!=1:
        raise ValueError("Unexpected upstream version: "+needle[:60])
    return hay.replace(needle,replace,1)
s=inject(s,'#include "plaits_move_engine.h"',
    '#include "plaits_move_engine.h"\n// TROLLfx chakra v0.1\n'
    'static const float chakra_hz[7]={396.f,417.f,528.f,639.f,741.f,852.f,963.f};\n')
s=inject(s,'    float notes_last;   // NOTES: the last position the host asked for (-1 = none yet), see set_param',
    '    float notes_last;   // NOTES: the last position the host asked for (-1 = none yet), see set_param\n'
    '    int chakra_root;\n    bool chakra_gate;')
s=inject(s,'    inst->notes_last = -1.0f;',
    '    inst->notes_last = -1.0f;\n    inst->chakra_root=3;\n    inst->chakra_gate=false;')
helper='''
static void chakra_off(freak_instance_t* inst) {
    if (inst->chakra_gate) inst->engine.note_off(69);
    inst->chakra_gate=false;
}
static void chakra_on(freak_instance_t* inst) {
    if (inst->chakra_gate) return;
    inst->params.pitch=12.0f*log2f(chakra_hz[inst->chakra_root]/440.0f);
    inst->params.amp_mode=PPF_AMP_GATE;
    inst->engine.set_params(inst->params);
    inst->engine.note_on(69,0.8f);
    inst->chakra_gate=true;
}
'''
s=inject(s,'static void set_param(void *instance, const char *key, const char *val) {',
    helper+'\nstatic void set_param(void *instance, const char *key, const char *val) {')
s=inject(s,'    if (strcmp(key, "state") == 0) {',
    '''    if (strcmp(key,"chakra_root")==0) {
        int idx=atoi(val);
        if(idx<0 || idx>6) return;
        bool held=inst->chakra_gate;
        chakra_off(inst);
        inst->chakra_root=idx;
        if(held) chakra_on(inst);
        return;
    } else if (strcmp(key,"chakra_gate")==0) {
        if(atoi(val)) chakra_on(inst); else chakra_off(inst);
        return;
    } else if (strcmp(key, "state") == 0) {''')
s=inject(s,'    if (strcmp(key, "name") == 0) return snprintf(buf, buf_len, "MPC Plaits");',
    '''    if (strcmp(key, "name") == 0) return snprintf(buf, buf_len, "TROLLfx Chakra Plaits");
    if (strcmp(key,"chakra_root")==0) return snprintf(buf,buf_len,"%d",inst->chakra_root);
    if (strcmp(key,"chakra_gate")==0) return snprintf(buf,buf_len,"%d",(int)inst->chakra_gate);''')
p=j["capabilities"]["chain_params"]
if any(x["key"]=="chakra_root" for x in p): raise SystemExit("Chakra already exists")
p.extend([
 {"key":"chakra_root","name":"CHAKRA","type":"enum","options":
  ["Root 396","Sacral 417","Solar 528","Heart 639","Throat 741","Third Eye 852","Crown 963"],"default":3},
 {"key":"chakra_gate","name":"TRIGGER","type":"enum","options":["Off","On"],"default":0}
])
l += '''
[tab CHAKRA]
art file=images/bg_voice.png fit=stretch
popup cx=370 cy=285 w=440 h=58 label="CHAKRA" key=chakra_root wheel=1
toggle cx=900 cy=285 label="TRIGGER" key=chakra_gate img=images/led_off.svg img_on=images/led_on.svg
knob cx=185 cy=525 r=44 label="HARMONICS" key=harmonics img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=475 cy=525 r=44 label="TIMBRE" key=timbre img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=765 cy=525 r=44 label="MORPH" key=morph img=images/knob_big.svg base=images/knob_shadow.svg
knob cx=1065 cy=525 r=44 label="OUT / AUX" key=aux_mix img=images/knob_big.svg base=images/knob_shadow.svg
qlinks "CHAKRA" = chakra_root,chakra_gate,harmonics,timbre,morph,aux_mix
'''
v.update({"name":"TROLLfx Chakra Plaits","vendor":"TROLLfx","uid":"TcPl","so":"chakra_plaits.so"})
j.update({"name":"TROLLfx Chakra Plaits","id":"trollfx-chakra-plaits"})
# All edits validated in memory before writing any of the files.
fp.write_text(s)
jp.write_text(json.dumps(j,indent=2)+"\n")
lp.write_text(l)
vp.write_text(json.dumps(v,indent=2)+"\n")
print("Applied Chakra tab, gate and root selection. Build via upstream ./build.sh")
