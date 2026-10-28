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
  in the end of `third-party/stb_image.h`. PNG decode only in this application.
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
