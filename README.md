# Java3DS — private alpha 0.3.0

Java ME / JAR / JAD launcher for Nintendo 3DS, with a shared MIDI synthesizer.
Extract the installation ZIP to the root of the SD card, replacing
`/3ds/java-3ds/java_pocket.3dsx`. The filename and data directory remain compatible
with Java Pocket 3DS. Games, settings and saves are retained. Start **Java3DS**
from Homebrew Launcher; add your JAR/JAD files under `/3ds/java-3ds/games/`.
A JAD also needs its matching local JAR. Original game JARs are not bundled.

## This update

* Private Wi-Fi release checks and installation from `Faefolk333/java3ds`.
  Startup check when an SD token exists; Y in the library opens Updates.
  Read **[UPDATES.md](UPDATES.md)** for the one-time read-only token setup.
* Per-game Controls menu: presets, hide the lower screen during play, remap
  fourteen physical controls, and reset custom bindings. START + SELECT
  always returns to the library. Hidden touch keys do not accept input.
* Centered touch keypad, pressed-key feedback, subtle parallax background,
  animated Y2K loading, English interface and Java3DS branding.
* Rendering serialized with the game's repaint calls; recursive repaint
  callbacks are guarded. This addresses races between mutable image buffers
  and painting. Soul of Darkness reached its title and in-game scene in host tests.
* Two display choices: **Native** and **3DS screen**. Native centers the original
  image and only shrinks if necessary. Generic games keep their original canvas
  in both modes; the 3DS screen mode scales the completed image to fit while
  preserving proportions, so black margins can remain. It does not invent a
  wider game camera or stretch people sideways.
* The known Prince of Persia Classic profile retains its actual 400×240 camera.
  Old and New 3DS have the same 400×240 2D output; there is no separate resolution
  switch for the model. Generic wide-camera adaptation remains game-specific.
* Nokia DirectGraphics short/int pixel read/write support for Might and Magic II.
* Cooperative VM stop at Java loop backedges, monitor contention, sleep and
  wait; monitor cleanup and daemon-thread teardown before VM memory is freed.

Native resolution comes from a `[320x240]` style filename hint or supported
Nokia display metadata. Without a reliable hint, 320×240 is labelled assumed.
Rename a JAR to include its known original dimensions when necessary.

## Controls and audio

Library: directions select, A opens, B goes back, X refreshes, Y opens updates,
START exits. Controls and bindings are saved per game.
Phone preset: directions, A/FIRE, B/0, SELECT/START softkeys, X/*, Y/#,
L/1, R/3, ZL/7, ZR/9. Numeric preset uses 2/8/4/6/5 for directions/FIRE.
PoP preset: B + direction jumps; A/R performs action/combat. Direction alone
runs or crouches. All twelve phone keys are available on the touch screen.
Custom bindings override individual buttons and can be reset to profile defaults.

Built-in SoundFont MIDI synthesis and PCM WAV playback use the DSP; the console
needs its usual `/3ds/dspfirm.cdc`. The MIDI engine is shared by games.

## Compatibility and validation

This is a partial MIDP runtime, not a universal Java or Android emulator.
The supplied Blades and Magic K800i/K790i JAR needs Mascot Capsule Micro3D v3;
that 3D backend is still unsupported and is reported before launch.
Some MIDP form APIs and game-specific behaviors remain incomplete.

See [VALIDATION.md](VALIDATION.md) for actual host checks and hardware limits.
Physical-console rendering, performance and Wi-Fi installation must still be
verified on the user's 3DS. No claim of complete compatibility with every JAR.
After a problem, copy `/3ds/java-3ds/runtime.log` and `output.log` before restarting.

## Building

JDK 17 (set JAVA_HOME), PowerShell: `./build-java.ps1`.
In devkitPro MSYS2: `bash build-runtime.sh` then `bash build-native.sh`.
Install external devkitPro packages `3ds-curl`, `3ds-mbedtls`, `3ds-zlib`.
Host VM: `bash build-host.sh` (MinGW). Package with `py tools/package_release.py`.
Private game-derived compatibility data and component licenses are described
in [THIRD-PARTY.md](THIRD-PARTY.md). Keep this repository and its releases private.
