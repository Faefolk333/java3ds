# Java3DS — private alpha 0.3.3

Java ME / JAR / JAD launcher for Nintendo 3DS, with a shared MIDI synthesizer.
Extract the installation ZIP to the root of the SD card, replacing
`/3ds/java-3ds/java_pocket.3dsx`. The filename and data directory remain compatible
with Java Pocket 3DS. Games, settings and saves are retained. Start **Java3DS**
from Homebrew Launcher; add your JAR/JAD files under `/3ds/java-3ds/games/`.
A JAD also needs its matching local JAR. Original game JARs are not bundled.

## Changes in 0.3.3

* Handles failed VM object/array allocations and retries after GC. Dump 26
  identified an unchecked allocation failure in Age of Heroes III.
* Native drawRGB, text and pixel reads avoid temporary full-frame images;
  queued repaint and one VBlank per frame reduce rendering overhead.
* Long sprite atlases (including Worms bodies) now decode and draw; PNG/JPEG
  dimensions are bounded by both length and total pixel count.
* MIDI/WAV players mix independently. Starting SFX no longer stops another
  supported music player. AMR, AAC and MP3 remain unsupported.
* LGE native-resolution metadata, functional RMS enumeration, Form/Gauge,
  and deferred callSerially callbacks improve compatibility.
* Experimental software Micro3D v3 and M3G scene rendering. Blades and Magic
  and Outland rendered 3D scenes in PC tests; Gladiator loaded animated models.
  These are initial backends, with incomplete effects and unmeasured 3DS speed.
* HID accelerometer/gyroscope bridges for JSR-256 and Sony sensor APIs.
  A game must actually request these APIs. The supplied LocoRoco Hi build does
  not reference them; adding sensors does not invent tilt controls for it.
* Touch keys highlight only on touch; menu parallax updates more often and
  filters the stick movement. The banner is unchanged.

## CIA installation (0.3.3)

Install `java3ds.cia` with FBI to launch directly from HOME Menu. The package
includes a native animated 3D banner converted from the supplied Blender scene
and all runtime/MIDI resources. CIA and 3DSX share `/3ds/java-3ds` on SD.
See [CIA.md](CIA.md) for local Wi-Fi QR installation and rebuilding the banner.
CIA updates select `java3ds.cia`; 3DSX updates select `java3ds.3dsx`.

## Launcher features (since 0.3.0)

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
* Two display choices: **Native** and **Fill screen**. Native centers the original
  image and only shrinks if necessary. Fill screen scales the completed canvas
  to 400×240 with no launcher-added margins; differing aspect ratios stretch.
  Borders drawn inside a game itself remain part of its image.
* The known Prince of Persia Classic profile retains its actual 400×240 camera.
  Old and New 3DS have the same 400×240 2D output; there is no separate resolution
  switch for the model. Generic wide-camera adaptation remains game-specific.
* Nokia DirectGraphics short/int pixel read/write support for Might and Magic II.
* Cooperative VM stop at Java loop backedges, monitor contention, sleep and
  wait; monitor cleanup and daemon-thread teardown before VM memory is freed.

Native resolution comes from a `[320x240]` style filename hint or supported
Nokia or LGE display metadata. Without a reliable hint, 320×240 is labelled assumed.
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
Micro3D v3 and JSR-184/M3G are experimental software backends. Micro3D command
lists, lighting/toon/sphere effects and some vendor APIs remain incomplete.
Some MIDP forms, Bluetooth, file connections and compressed audio codecs also
remain incomplete. See [COMPATIBILITY.md](COMPATIBILITY.md) for game evidence.

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
