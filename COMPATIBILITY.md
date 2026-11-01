# Compatibility work — 0.3.8

The user's 0.3.3 console report: Blades 240×320 launches slowly; the 320×240
variant crashes the system; Outland launches but is impractically slow; Worms
fails to load. These reports take precedence over PC startup smoke success.
Dump 28 names Java3DS. PC 0x0014CE50 / LR 0x001504F0 match the saved 0.3.3 ELF: utf8_append_part_c / _parseCPString. The dump instruction bytes match the ELF; r4 is NULL and the fault address is 4. utf8_create failed during class metadata parsing. Parser allocations now check failure and propagate an abort through class loading/JNI. A forced failure produces a clean VM error and teardown instead of the native NULL access. This does not prove every VM OOM path is covered. Existing dump 27 names "menu" and is separate.

M3G now uses native nearest-texture scanlines for supported compositing paths,
including vertex colors, depth, alpha and fog. Unsupported filters/multitexture
still use the Java reference path. A mixed S/T wrapping bug in that reference
path was also fixed. A differential 90-scene PC test compares native and Java
output; twelve exponential-fog pixels differ by at most one channel level
(two with MODULATE_X2). The measured run was 641ms Java versus 146ms native.
This is a PC test, not a measured console FPS gain.

Real Outland's 30-second PC test recorded 73,570 native spans, 701,547 native
pixels and zero Java fallback pixels, then exited 0. The sampled cave renders.
Blades now transforms model vertices and bone poses in C, reuses work arrays,
and preserves the extra sentinel entries used by the model buffers. Both
variants rendered the forest/player in 45-second PC runs and returned 0.
A sentinel-tail and invalid bone-count regression passes. The targeted allocation fix and playable speed still need a console retest.

Orcs & Elves II's real JAR was valid. It failed loading the missing WMA
TextMessage type and Alert APIs. Offline messaging interfaces and basic alerts
now let it reach the first-person dungeon/tutorial; no SMS transport is provided.
Worms reached gameplay with complete bodies in a final 45-second PC run; the console
load regression is still unverified. Log rotation retains the previous run.

Spore Creatures in D:/HRY JAVA is a 454-byte textual JAD pointing to a remote
download endpoint, with no matching 687220-byte game JAR; the user confirmed
there is no other JAR. This cannot be run as a game. Catalog messages now
separate descriptors from ZIP payloads, handle double extensions and inspect
ZIP content before treating a .jad filename as a descriptor.

Touch softkeys, a cooperative VM/audio pause menu, scaling changes and backlight
sleep were added. The first touch wakes without activating a game key. Native
scaling changes apply immediately; changing a PoP camera profile requires a
relaunch. Hardware touch/backlight/audio/exit checks remain pending.

MIDI memory: engines now share a lazily loaded bank; WAV, unsupported audio and
MIDI parsing do not load it. SF2 samples stay PCM16 instead of being expanded
into float storage. A 256 KiB lookup preserves the original float sample values;
all eleven MIDI/WAV render hashes match the previous engine. Blades avoids the
old ~31 MiB bank allocation entirely. Games that play MIDI use about half that
sample RAM. Clone lifetime and independent mixing remain tested.

## Earlier 0.3.3 work

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

## 0.3.6 renderer and stereo

Blades uses cached native triangle batches and target-sized depth clears. A PC
35-second comparison produced 602 reference / 726 batched frames with about 20%
less CPU time; physical speed remains unmeasured. The game menu adds optional
model/layer depth reprojection for stereo output. It supports direct-canvas
Micro3D/M3G and explicit LayerManager ordering, keeps later HUD drawing flat,
and fills disocclusion gaps approximately. Mutable Images, clones and getRGB/drawRGB preserve depth through a bounded
readback cache. Arbitrary game-side array copies can still lose it. This is not a GPU renderer or universal automatic
2D-to-3D conversion. See validation/0.3.6 and README.

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

## 0.3.7 follow-up

Moorhuhn's missing foreground was caused by repainting its GameCanvas twice;
the corrected host run reaches menu and shooting with complete sprites.
Orcs & Elves II also uses GameCanvas; publishing the buffer directly avoids
that duplicate paint path. Physical menu flicker needs retesting.
Blades 320x240 uses both MIDI and AMR-NB; both now decode, and a fresh-save
host run selecting sound Yes starts MIDI and produces nonzero PCM.
Soul loads/starts supported media. Its supplied bytecode disables many effects
with sound ID -1 and changes requested infinite loops to one playback; this
accounts for some silence. These game decisions are not overridden.
Yeti golf draws its club as a five-degree arc, now implemented correctly.

Micro3D blending is per mesh; globally sorted translucent geometry and full
lighting fidelity are incomplete. Stereo layer hints work on ordinary image
and sprite draws, not arbitrary flattened pixel arrays. Orcs' custom software
3D renderer exposes no Micro3D/M3G depth. No raycaster-specific adapter exists.
CRT filters are presentation-only. All new changes require device validation.

## 0.3.8 presentation/performance follow-up

Opaque GameCanvas color/depth rows publish directly. Six scripted games still
reach rendered scenes and exit normally. Host transfer benchmarks and device
stub checks cover the optimization and New-core-2/denied-core/Old fallback.
Actual 3DS frame rates are not yet measured. Four stronger CRT masks are
presentation-only; the UI remains unfiltered. Banner 0.3.7's parent billboard
failed on console; 0.3.8's timed counter-orbit is experimental and not yet a
confirmed fix. See CIA.md for its timing limitation.
