#!/usr/bin/env bash
set -euo pipefail
export PATH="/mingw64/bin:$PATH"
gcc -O2 -c third-party/minijvm/minijvm/c/utils/miniz.c -o work/miniz-host.o
g++ -std=c++17 -O2 -Ithird-party/minijvm/minijvm/c/utils native/catalog.cpp tests/catalog_probe.cpp work/miniz-host.o -o work/catalog-probe.exe
