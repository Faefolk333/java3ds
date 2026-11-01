# Components and private compatibility data

* miniJVM: bundled modified C runtime and bootstrap binary; upstream Gust,
  https://github.com/digitalgust/miniJVM . See `third-party/minijvm/LICENSE`.
  Existing local modifications include offline operation, ARM platform glue,
  GC/thread fixes, interpreter instrumentation and resource ZIP caching.
* TinySoundFont / TinyMidiLoader: Bernhard Schelling, MIT,
  https://github.com/schellingb/TinySoundFont , revision in
  `third-party/TinySoundFont/REVISION.txt`; headers and licenses supplied.
* GeneralUser GS 2.0.3: S. Christian Collins, supplied license in
  `third-party/GeneralUser-GS/`. Derived bank retains all 287 presets;
  samples were downsampled for this build. Reproduction script and checksums
  in `tools/prepare_bank.py` and `third-party/GeneralUser-GS/mobile-bank.json`.
  https://schristiancollins.com/generaluser.php
* stb_image: Sean Barrett and contributors, MIT/public-domain dual license
  in the end of `third-party/stb_image.h`. PNG and JPEG decoding in this application.
  https://github.com/nothings/stb
* font8x8_basic: public-domain font data, https://github.com/dhepper/font8x8 .
  Original header and pinned revision supplied in `third-party/font8x8/`.
* devkitARM/libctru: external build dependencies, not newly vendored.
  https://github.com/devkitPro/libctru

`romfs/profiles/pop-classic.zip` contains modified game classes and a modified
text resource derived from the user's Prince of Persia Classic JAR, rather
than an independently licensed universal emulator component. `pop-theme.mid`
is the game's original music. These remain private compatibility data;
no redistribution license to those game assets is asserted. The original game
JAR, graphics and levels are not part of the install archive.

`LauncherCheck` and its test melody are original diagnostic content for this project.

* libcurl 8.4.0 and mbedTLS 2.28.8: devkitPro 3DS portlibs; external static libraries used by the updater. Licenses copied to third-party/curl/LICENSE and third-party/mbedtls/LICENSE. https://curl.se / https://github.com/Mbed-TLS/mbedtls
* The HTTPS CA bundle contains public USERTrust ECC and ISRG Root X1 certificates; provenance and fingerprints in UPDATES.md. zlib is an external devkitPro dependency (zlib license).

## Additional APIs and 3D backends (0.3.3)

Micro3D API, MBAC/MTRA decoders, animation utilities and sensor API types are
adapted from individually Apache-2.0 licensed J2ME-Loader source files. Original
headers are retained; see third-party/J2ME-Loader-API/LICENSE and NOTICE.

M3G scene graph, loader and software renderer are adapted from FreeJ2ME-Plus
under GPL-3.0-or-later. Original headers are retained. See
third-party/FreeJ2ME-M3G/LICENSE and NOTICE. Complete corresponding modified
source and build scripts are in this private repository and source ZIP.

## User-supplied UI assets (0.3.4)

The new icon and Blender-derived banner use the user's Desktop artwork.
The three UI WAVs were supplied in the user's "PSP Sounds" folder under names
recorded in assets/ui-sound-source.json; no ownership or redistribution license
is asserted. They remain private data in this repository and local packages.
M3G native span code ports the existing GPL-3.0-or-later pipeline; its original
license and corresponding source are included. Micro3D native matrix/pose code
replaces the prior Java platform bridge.

TinySoundFont is locally modified with optional PCM16 sample storage and an exact
short-to-float lookup to reduce RAM. Original MIT license is retained.
