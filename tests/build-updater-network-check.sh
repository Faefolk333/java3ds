#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
export PATH="/mingw64/bin:$PATH"
bash tests/build-updater-check.sh
MBED=third-party/minijvm/minijvm/c/utils/https/mbedtls
g++ -std=c++17 -O2 -Itests/updater_stubs -I"$MBED/include" '-DJAVA3DS_UPDATE_VERSION="0.3.9"' '-DJAVA3DS_UPDATE_CA_FILE="romfs/certs/github-roots.pem"' tests/updater_network_check.cpp native/updater.cpp work/sha-host.o -lcurl -lpthread -o work/updater-network-check.exe
