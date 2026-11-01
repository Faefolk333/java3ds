# Compatibility work — 0.3.3

## New Age of Heroes III dump

Dump 00000026 is Java3DS 0.3.2. Against the preserved matching ELF,
PC 0x001333B8 maps to jarray_create_by_class. A 19200-int allocation
(76840 bytes including the object header) returned NULL, then the VM wrote
through it. This is distinct from the older missing Nokia API failure.
Object/array creation now checks allocation/size overflow, retries after a GC
safe point and reports Java OutOfMemoryError; a small reserve supports exception
creation. A host fault-injection test rejects this same array allocation twice,
and both VM runs return normally after catching OutOfMemoryError.
This verifies the failure path, not adequate RAM for every level on hardware.

## Rendering, speed and audio

Native RGB/font/readback paths avoid Java pixel-array churn; decoding no longer
copies through an extra complete pixel array. Generic repaint is deferred to
the display pump; self-rescheduling serial callbacks wait until the next pump.
Frame scaling uses precomputed coordinates and tiled output. Only one VBlank
wait remains per game frame, and the touch keypad is not repainted for every
published game frame. Interpreter builds use O2 and disable costly per-call
PoP diagnostics while retaining thread/GC failure diagnostics.
A PC RGB/text microbenchmark produced the identical image hash in 10ms versus
393ms for the prior path. This is not a whole-game or physical 3DS speed claim.

Worms built long sprite atlases rejected by the former 1024-pixel dimension cap.
The decoder and image/region bridges now allow up to 16384 on one axis while
capping images at 1048576 total pixels. The final host gameplay frame shows
complete worms, instead of just helmets. PNG and JPEG are supported.

Independent MIDI synthesizer state shares the SoundFont sample bank. MIDI and
PCM WAV slots mix without cancelling each other. Codec support is unchanged
for compressed formats: AMR/AAC/MP3 assets may remain silent.

## Games and device APIs

LocoRoco Hi declares LGE-MIDlet-TargetLCD-Width/Height 240×320. This now overrides
the assumed 320×240 default. Native centers/fits the image; Fill screen stretches
the complete canvas to 400×240. The launcher adds no margins in Fill screen,
but cannot distinguish baked game artwork from its own internal borders.

JSR-256 acceleration and angular_velocity sensors, plus Sony accelerometer
connections, use cached real HID samples on 3DS. Host tests use explicit fixture
values; physical axes/calibration still need testing. Game Connector calls are
redirected to the app-visible connection factory. The supplied LocoRoco build
has no references to these sensor APIs, so it still uses its own original input.

RMS enumeration/filter/comparator, correct missing-store exceptions, basic
Form/StringItem/Gauge and Image InputStream improve load/save compatibility.
Discworld reached its intro dialogue in a 25-second PC test. Several other
forms, vendor APIs, file/Bluetooth connections and audio controls remain missing.

## Experimental 3D

Micro3D v3: real MBAC/MTRA decode, poses, textured software triangles and depth.
Blades and Magic rendered forest geometry, the player and a boar on PC.
Command lists, lighting, toon/sphere effects and some multi-transform behavior
are incomplete. M3G: scene graph, assets, transforms, animation and software
renderer adapted from FreeJ2ME-Plus. A loader section-boundary fix allows the
final root Group/Mesh section in Outland's real prize.m3g asset to load.
Outland rendered its 3D cave; Gladiator loaded skinned models and animations.
Neither backend is GPU accelerated, and 3DS frame rate is unmeasured.

Symbian emulation and banner framing/motion remain separate later work.

## Historical 0.3.2 diagnosis

Supplied Luma dumps 00000023/24/25 name Java3DS, title 000400000F3A3D00.
All report prefetch abort at PC 001EF8B4, LR 001EF1B4. Before relinking the
matching local CIA ELF, addr2line mapped these to svcBreak and
__system_allocateHeaps (libctru allocateHeaps.c:66). Disassembly places LR after
the panic branch for failed regular-heap svcControlMemory. The failing size
visible in registers is 0x07328000 (about 115 MiB). The new startup hook bounds
regular heap to 48 MiB (Old) / 80 MiB (New), 4 MiB linear, leaves commit margin,
and falls back to libctru defaults when resource-limit queries fail.
This is a targeted fix for these startup dumps, not proof that the original
Dark Knight runtime crash is fixed. Dumps 00/01 name LEGOLOTR; 02 names biorev3D.

The supplied output.log names Assassin's Creed Brotherhood and reports
IOException: Invalid RMS name. Original simple filenames remain readable.
Other valid names are encoded as UTF-16 hex into a single safe filename.
LocoRoco references listRecordStores, which is now implemented, and empty
records with null data no longer cause arraycopy NullPointerException.

Age III's missing Nokia Sound API led to a native NULL dereference while
resolving an array component class. The facade routes MIDI/WAV through the
shared player and supports generated frequency tones. Proprietary Nokia tone
sequence formats are not fully implemented. Array resolution now fails with
a Java exception when an optional device class is absent.

GameMain no longer holds its class monitor while calling destroyApp(true).
A callback that starts/joins another shutdown caller can now complete. The
existing cooperative VM stop remains; physical START+SELECT needs retesting.
