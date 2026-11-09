#!/usr/bin/env python3
"""Apply the reviewed Chakra Plaits integration to a clean upstream checkout."""
import pathlib
import subprocess
import sys

if len(sys.argv) != 2:
    raise SystemExit("Usage: add_chakra_page.py /path/to/mpc-vst-plaits")

root = pathlib.Path(sys.argv[1]).resolve()
patch = pathlib.Path(__file__).with_name("chakra-plaits.patch")
required = [root / "src/dsp/freak_plugin.cpp", root / "vst/vst.json", patch]
for path in required:
    if not path.is_file():
        raise SystemExit(f"Missing {path}")

check = subprocess.run(["git", "apply", "--check", str(patch)], cwd=root)
if check.returncode:
    raise SystemExit("Patch does not apply cleanly; use poloq mpc-vst-plaits commit b113c83")
subprocess.run(["git", "apply", str(patch)], cwd=root, check=True)
print("Applied TROLLfx Chakra Plaits integration and tests")
