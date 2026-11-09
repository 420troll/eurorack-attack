#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENDOR="${ROOT}/vendor/eurorack"
if [[ ! -f "${VENDOR}/plaits/dsp/voice.h" ]]; then
  mkdir -p "${ROOT}/vendor"
  git clone --recursive https://github.com/pichenettes/eurorack.git "${VENDOR}"
fi
git -C "${VENDOR}" submodule update --init stmlib
cmake -S "${ROOT}" -B "${ROOT}/build" -DPLAITS_ROOT="${VENDOR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${ROOT}/build" -j "$(nproc)"
ctest --test-dir "${ROOT}/build" --output-on-failure
"${ROOT}/build/chakra_native_render" "${ROOT}/build/heart_chord.wav" 639 chord 8
echo "Reference WAV: ${ROOT}/build/heart_chord.wav"
