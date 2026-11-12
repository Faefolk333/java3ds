# Java3DS — test build 0.4.0

The separate Symbian diagnostic now links owner-supplied EKA1 game DLLs and
all their imports recursively, before running any application code. With the
supplied RH-29/Shadowkey dataset, the host test resolves all 415 main imports
and 284 imports from GAMECOMMS/SIMKIN; both game DLL thread-attach entries
return successfully. A checked GCC2 primary application vtable call returns
Shadowkey's actual application UID. These new steps need console validation.

Update the main app, reopen it once, and unpack the new PRIVATE Shadowkey test
ZIP locally onto SD, then run Symbian Probe 0.4.0 -> Y. The private ZIP adds
game-libs/GAMECOMMS.DLL, game-libs/SIMKIN.DLL and eight matching ROM headers.
No firmware/game data is uploaded or bundled with the public release.

**This remains a diagnostic, not a playable Symbian launcher.** Complete ROM
DLL initialization, application framework/process environment, scheduling,
filesystem/window/audio services, EKA2 and SIS installation remain incomplete.
The previous 0.3.15 heap/application factory path passed on the owner's New 3DS.
Java ME behavior is unchanged. See SYMBIAN.md and VALIDATION.md.

## Previous history

# Java3DS — test build 0.3.15

Java ME / JAR / JAD launcher for Nintendo 3DS, with a shared MIDI synthesizer.
Extract the installation ZIP to the root of the SD card, replacing
`/3ds/java-3ds/java_pocket.3dsx`. The filename and data directory remain compatible
with Java Pocket 3DS. Games, settings and saves are retained. Start **Java3DS**
from Homebrew Launcher; add your JAR/JAD files under `/3ds/java-3ds/games/`.
A JAD also needs its matching local JAR. Original game JARs are not bundled.

## Changes in 0.3.15

Add the first single-thread EPOC6 executive model to the separate Symbian probe:
HAL page size, anonymous normal chunks with committed-page checks, per-thread
heap selection, bounded handles, request completions, recursive mutexes,
semaphores and serial locked increment/decrement. Blocking calls stop with a
scheduler-required result. Unsupported functions remain explicit stops/errors.

Use EUSER's real ChunkHeap/HeapSwitch code from the supplied matching ROM to
create/select RHeap before calling the application's factory. On the PC,
Shadowkey now returns an application object with a mapped virtual table instead
of stopping on Heap SVC 0x6c. The owner's New 3DS logs now confirm this result:
all CPU/E32/kernel fixtures pass and the full diagnostic completes in 3613 ms.
This does not measure gameplay performance. Evidence: validation/0.3.15.
Original CPU tests cover ARM/Thumb SVC return via LR, chunks, stale handles,
synchronization and heap context. No firmware or game is bundled.

Update the main app, reopen it once, then run Symbian Probe 0.3.15 -> Y with the
same private SD data. Kernel check results go into symbian-probe.log; the heap
bootstrap and application-object result go into symbian-loader.log.
**There is still no phone OS boot, game window, SIS installation, graphics/audio
services, full scheduler or complete Symbian game support.** Java behavior is
unchanged. Details: SYMBIAN.md.

## Earlier changes in 0.3.14

The owner's New 3DS logs confirm the 0.3.13 startup fix and all CPU/E32 fixture
checks, including relocation, guarded memory, instruction limits and cancellation.
The real Shadowkey E32 also validates/maps. Its ROM read was cancelled by START,
as confirmed by the owner; full ROM linking has not yet been verified on console.

Show ROM/program read progress, KiB and elapsed time in the separate probe.
Read ROM in 64 KiB blocks. Share a latched task cancellation across I/O, linking
and execution so a cancelled ROM read cannot fall through to guest calls.
The log records the stage, bytes and elapsed time at cancellation. B cancels the
image diagnostic and Y retries; START exits. New controls/UI need a device test.
The same private SD data remains usable. No new kernel/gameplay support is claimed.

## Earlier changes in 0.3.13

Fix standalone 3DSX startup on a 124 MiB Homebrew Launcher process. Dump 31's
code window matches the 0.3.12 probe ELF exactly: svcBreak is called by the SDK
heap allocator before main. With a fixed 2 MiB linear heap, the old startup
requests about 121.59 MiB of regular heap, beyond its 96 MiB address area.

The existing CIA bounded-heap startup hook is now linked into both the main
3DSX and Symbian probe as well. It caps regular memory at 80 MiB on the larger
profile / 48 MiB on the smaller profile, uses 4 MiB linear memory and retains
resource-limit headroom. Tests reproduce the dump's allocation inputs and
inspect all three final ARM ELF startup call targets. Physical startup needs a
retest. EKA1 loader behavior and the private SD test data are unchanged; there
is still no playable Symbian runtime. Run the updated probe, then Y for the
previously prepared optional SD diagnostic.

## Earlier changes in 0.3.12

Fix automatic standalone-probe extraction: validate 3DSX and SMDH assets using
their own formats instead of rejecting them as non-JAR files. Opening the
updated main app installs the probe into Homebrew Launcher.

Extend the separate Symbian probe with a bounded ARM EKA1 E32 loader: checked
UIDs, code/data/BSS, relative exports, imports, relocations, sparse 24 MiB guest
memory, RX code and RW/non-executable data/stack. Add an uncompressed EKA1 ROM
mount with exact matching DLL descriptors and direct ordinal linking. Missing
imports, SVCs, bad accesses, cancellation and instruction budgets stop with a
specific result. They do not return fabricated system-service success.

The CPU foundation passed its small hardware checks on the owner's New 3DS in
0.3.11. New E32/ROM integration is host-tested and needs a console test. With the
owner's Shadowkey/QD image, 324/415 imports link to real ROM code; the application
factory reaches EKA1 SVC 0x6c (Heap), which is not implemented yet. **This is
not phone OS boot, playable Symbian support or SIS installation.**

In the probe, **Y** diagnoses optional owner-supplied files under
`/3ds/java-3ds/symbian/`: `test.app`, `SYM.ROM` and matching `rom-libs/*.DLL`
headers. It saves `/3ds/java-3ds/symbian-loader.log`. No ROM/game is bundled or
downloaded. Local `tools/prepare_symbian_test.py` can prepare a private SD ZIP
from the owner's files; never upload that ZIP. Details: SYMBIAN.md.

## Earlier changes in 0.3.11

Fix the console VM clock epoch. libctru osGetTime starts in 1900; Java and
pthread CLOCK_REALTIME deadlines start in 1970. Passing the raw console time
could turn a short monitor/condition timeout into a distant or overflowing
deadline, causing console-only hangs. The VM now converts to Unix milliseconds.
A host build emulating Nintendo's clock passes 22 runtime probes twice,
including independent native-clock comparison and real timed waits. APT/UI
relative timers retain their existing clock.

Targeted host probes with the emulated console clock reach Gladiator combat,
LOTR gameplay, Orcs & Elves II's world and Resident Evil: The Missions item
selection beyond its reported mission screen. Both supplied Orcs II filenames
are byte-identical. Supplied console logs do not capture the latest reported
four failure points; a physical 3DS retest remains necessary. Save files are
preserved. This is a timing fix, not certification of every game/save state.

Initial Symbian implementation is a **separate CPU/input probe**, included in
the SD installation ZIP. Run Java3DS Symbian CPU Probe from Homebrew Launcher;
the published 0.3.11 needs a manual standalone 3DSX copy because of its installer
format bug. Automatic extraction is fixed in 0.3.12.
The probe saves `/3ds/java-3ds/symbian-probe.log`. It ports EKA2L1 dyncom to ARM11,
uses an 8 MiB cache, checks ARM/Thumb/VFP/context/exclusives and previews physical
controls mapped to a virtual touchscreen. **It does not boot a phone OS or run
SIS/SISX/APP games.** Kernel/loaders/services/graphics/audio integration remain
to be ported. See SYMBIAN.md. No firmware or game payload is bundled.

## Earlier changes in 0.3.10

VM/Java/GC thread priorities now agree and stay below APT's event thread.
Recursive Object.wait releases/restores every monitor level. HOME/lid hooks
only update atomic suspend flags; VM/audio pause independently of the game menu.
These address scheduling and GC waits observed in the supplied console logs;
physical HOME/lid behavior still needs a console retest.

Normal cancellation unwinds the VM. A native call that has not stopped after
five seconds triggers process recovery: CIA relaunches itself; 3DSX ends the
process. No live worker's memory is freed for reuse. Recovery is a fallback,
not a promise that every native call cancels instantaneously.

All three supplied LOTR variants default to portrait (use Scale to fit).
DawnStar and the supplied Elder Scrolls Travels have correct 176x208 profiles,
A/right-softkey and B/left-softkey menu confirmation, numeric game controls,
Unicode/Cyrillic text and a system keyboard for names. Both reached actual
dungeon gameplay in interactive host tests. Gladiator 3D reached combat.
Micro3D triangle/quad renderPrimitives now serves the supplied Devil May Cry;
this is partial API support, not a complete Micro3D implementation.

The updater no longer requires a token during authorized public test windows.
Install this build once manually when coming from token-less 0.3.9. See
[UPDATES.md](UPDATES.md). Startup displays runtime preparation progress.
Symbian files were inventoried, but a Symbian execution backend is **not**
implemented; see [SYMBIAN.md](SYMBIAN.md).

## Earlier changes in 0.3.9

Cancellation requests now reach the VM immediately, including requests during
launch preparation. Interrupted native calls no longer treat an error status
as a Java exception reference. Forced exits skip user shutdown hooks and poll
sleep/monitor waits more frequently. MIDI seeks and AMR decoding also poll an
optional stop callback. Four real games each survived three cancelled VM starts
in one host process; seven one-shot cancellation probes returned about 0.15–0.30
seconds after the request. Console cancellation timing still needs verification.
Unchanged ZIP resources are copied in compressed form during preparation.

Display offers **Scale to fit**, **Fill screen** and **Native pixels**. Fit can
shrink portrait games to the display height without distorting proportions.
Phone screen orientation is separately Auto / Portrait / Landscape before launch.
The supplied LOTR Middle-earth Defence JAR requires portrait despite its filename.
The supplied Harem Adventures is a Siemens 120×160 build; its legacy key codes
are translated, and the host probe now reaches the first level.

TextField, ChoiceGroup, Spacer, ImageItem and basic Siemens UI adapters extend
compatibility. High-level forms/lists dispatch softkey commands and show their
labels. These are partial MIDP/vendor adapters, not the complete phone platform.
Elder Scrolls Travels completes dungeon loading and DawnStar passes its former
UI-allocation failure in host startup tests; complete gameplay is not certified.

The game menu has Original / Soft / Warm audio tone. Soft is the default and
reduces high-frequency output. It currently filters the final mono mix, including
effects. Original bypasses it. MIDI/PCM mixing remains independent.

Micro3D shares perspective reciprocals across the pixel loop. A 10,000-triangle
comparison found identical output over 40,960,000 pixels, 121 -> 106 ms host CPU
time. This is the renderer loop, not whole-game/device FPS. The supplied Orcs &
Elves 2 and Doom II RPG have opt-in wall-column stereo adapters. Both real JARs
produce depth in host scene probes. Floor/object depth remains approximate;
other JAR variants are not automatically assigned these profiles.

The banner follows the working **Faedrops 0.27** mesh-facing layout: initial
poses are baked into geometry, each actual mesh has YAxial facing, and relative
authored TRS channels keep the five-second loop. The failed timed eight-second
counter-rotation has been removed. There is no version badge. Binary dictionaries,
mesh bindings and serialized authored vertex motion pass; physical HOME Menu
playback and its handling of authored rotation remain unverified.

Known gaps remain: Spore Creatures only has a JAD on this PC, the supplied NFS
The Run depends on unavailable Reg/Util natives, and several slow/black-world
3D titles still need further work. See COMPATIBILITY.md and VALIDATION.md.

## Changes in 0.3.8 (historical)

Added stronger CRT Scanlines, RGB grille, Shadow mask and Color TV alongside
Soft lines / Soft grid. Improved opaque GameCanvas transfer and used New 3DS
application core 2 for audio with an Old/loader fallback. The timed banner
compensation attempted in that version failed the console test and is replaced.

## Changes in 0.3.7 (historical)

GameCanvas flush now publishes its buffer directly instead of calling the
application's paint method twice. Moorhuhn again renders its menu, chickens,
targets and crosshair in host gameplay tests. Hidden canvases cannot publish
frames, and selecting the same screen no longer triggers hide/show callbacks.

Micro3D implements half/add/subtract blending, color-key transparency and
opaque-before-transparent submission within each mesh. Neutral black additive
pixels no longer overwrite the scene or its stereo depth. Projection respects
translated/offscreen graphics. Early triangle rejection avoids unnecessary
per-pixel divisions while preserving opaque reference pixels.

AMR-NB effects use a reusable OpenCORE decoder. PlayerListener, mute, duration
and playback-position APIs improve game audio compatibility. Signed PCM
interpolation fixes distortion in negative WAV/AMR samples. MIDI, WAV and AMR
players mix independently; AAC/MP3 and AMR-WB remain unsupported. The supplied
Soul build disables many effects using ID -1 and forces infinite-loop requests
to one playback; some silence is a game-build choice.

Stereo defaults to **Auto** and stops collecting depth with the hardware slider
lowered. Models uses Micro3D/M3G depth. Layers combines LayerManager with an
experimental estimate for games drawing ordinary images/sprites; Auto enables
both. Small HUD text stays on the screen plane. This is image reprojection,
with approximate gap filling; it cannot recover hidden geometry. Custom games
that provide a flattened software-rendered framebuffer, including Orcs & Elves
II, still need renderer-specific depth integration. Depth from mutable images
and the bounded getRGB/drawRGB cache remains supported. Older default-Off
settings migrate to Auto once; a newly saved Off choice is retained.

L+R or touch MENU offers CRT **Off / Soft scanlines / Soft grid** (expanded in 0.3.8). Filters affect
only the game image, share the same phase in both eyes and preserve settings
per game. Yeti Sports' golf club now uses the requested five-degree fillArc
instead of an incorrectly filled oval.

The banner has no version badge. The attempted Y-axis billboard parent failed the subsequent console test;
0.3.8 replaces it with a separate orbit-compensation channel. Single-sided closed meshes reduce CGFX
from 502,712 to 264,072 bytes. The supplied quiet WAV is normalized to -1 dBFS
and resampled to 32 kHz; its original is retained. No significant leading
silence was present. HOME Menu controls startup timing. Motion, sound and
performance still require a console retest; PC results do not establish 3DS FPS.

## Changes in 0.3.6 (historical)

Native Nokia pixel copies and Micro3D mesh batching reduced host rendering
overhead. Experimental model/layer stereo and per-game strength settings were
introduced. The temporary version badge from that build is removed in 0.3.7.

## Changes in 0.3.5 — HOME Menu banner hotfix

Dumps 29/30 belong to HOME Menu, before Java3DS starts. The 0.3.4 banner had
an invalid skeleton name dictionary: translucent-node sorting moved entries
without rebuilding its Patricia branches. Rebuild that dictionary and explicitly
name rigid mesh nodes. The build now checks all 83 serialized name lookups;
the previous banner fails this check. The fixed-camera animation and glass
materials remain. The supplied 1.846-second stereo WAV is now embedded.

Install the replacement CIA with FBI, without selecting the broken Java3DS
HOME icon first. Keep title ID 000400000F3A3D00; games and SD saves are retained.
Physical HOME Menu playback still needs a console retest. The earlier local
0.3.4 QR server was stopped to avoid reinstalling the broken banner.

## Changes in 0.3.4

Touch LEFT/RIGHT softkeys and MENU join the phone keypad. MENU (or L+R)
pauses the VM and game audio, changes display scaling or sleep timeout,
returns to the library, or closes Java3DS. START+SELECT remains a direct exit.
The lower backlight sleeps after 30 seconds by default (Never/15/30/60/120);
the first touch wakes it without pressing a phone key. Settings persist per game.

M3G's common nearest-texture scanlines and Micro3D vertex/bone transformations
now run in native code. 3D remains experimental software rendering. Dump 28 identified a failed UTF-8
metadata allocation in Blades 320×240. Parsing now aborts safely on that failure;
the MIDI bank loads on demand and uses half the sample RAM. Physical crash
and performance retests remain pending. Orcs & Elves II reached gameplay on the PC after
adding offline messaging types and alerts. Compressed audio remains incomplete.

The supplied icon, fixed-camera five-second glass-style banner, navigation,
confirmation and update-success sounds are included. Runtime/output logs retain
one previous run. JAR/JAD labels and messages now distinguish descriptors from
actual game ZIPs, including binary files with a misleading .jad suffix.

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

## CIA installation (0.3.8)

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
