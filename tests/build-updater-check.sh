#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
export PATH="/mingw64/bin:$PATH"
mkdir -p work
MBED=third-party/minijvm/minijvm/c/utils/https/mbedtls
gcc -O2 -Inative -I"$MBED/include" -DMBEDTLS_CONFIG_FILE='"sha256_config.h"' -c "$MBED/library/sha256.c" -o work/sha-host.o
g++ -std=c++17 -I"$MBED/include" tests/updater_check.cpp work/sha-host.o -o work/updater-check.exe
./work/updater-check.exe
