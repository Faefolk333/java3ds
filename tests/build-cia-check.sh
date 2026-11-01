#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash tests/build-updater-check.sh
export PATH="/mingw64/bin:$PATH"
MBED=third-party/minijvm/minijvm/c/utils/https/mbedtls
g++ -std=c++17 -Itests/cia_stubs -I"$MBED/include" tests/cia_update_check.cpp work/sha-host.o -o work/cia-update-check.exe
./work/cia-update-check.exe
g++ -std=c++17 -Itests/cia_stubs tests/cia_heap_check.cpp native/cia_heap.cpp -o work/cia-heap-check.exe
./work/cia-heap-check.exe
