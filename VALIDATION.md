# Java3DS 0.3.13 validation — 2026-11-05

Physical startup and the new EKA1 loader still require a console retest.

* Dump 31's full 96-byte code window matches the 0.3.12 probe ELF. PC 0x13607c
  is svcBreak; LR 0x137e14 returns to SDK __system_allocateHeaps after its failed
  regular-memory allocation. This happens before main, not in guest execution.
  Observed regular heap size 0x7997000 = 121.59 MiB exceeds the 96 MiB area.
* Shared heap-policy checks reproduce max=124 MiB, used=0x69000, fixed-linear=
  2 MiB from the failed request. The wrapper instead requests 80 MiB regular /
  4 MiB linear. Checks also cover Old64, a constrained limit and API fallback.
* Actual linked startup call targets are checked for CIA, main 3DSX and probe:
  CRT -> bounded heap wrapper -> SDK. The same check rejects the old probe ELF.
* The EKA1 loader, ROM links and private SD inputs are unchanged. No new game or
  OS functionality is claimed. Prior host loader/ROM regression remains valid.
* Final CIA is title 000400000F3A3D00, version 61 / 0.3.13. CTRTool content
  hashes pass with the three expected unsigned signature failures. Extracted
  RomFS, icon, banner and original audio PCM match; embedded-asset and banner
  format regression checks pass.

## Previous validation history

# Java3DS 0.3.12 validation — 2026-11-05

New EKA1 loader/ROM execution on physical hardware is not yet verified. The
owner's New 3DS CPU-only 0.3.11 pass remains evidence for its small fixtures only.
Java timing/lifecycle hardware retests remain outstanding.

* Original E32 fixture executes through the real EKA2L1 CPU: checked UID,
  non-default load bases, text/data/inferred relocations, BSS zeroing, return
  value and data write; exact missing import and SVC stops; RX/NX protection,
  instruction limit and cancellation. 418 malformed/truncated variants reject.
* Original ROM fixture links an ordinal to real guest code and returns 77.
  Tests cover memory limits, wrong/truncated/unsupported device images, exact
  descriptor pairing, UID mismatch, read-only ROM and cancelled I/O rollback.
* Supplied Shadowkey links 324/415 imports from its matching RH-29 ROM. Its
  DLL entry returns; first export enters EUSER then stops at SVC 0x6c (Heap).
  Other supplied EKA1 image results are retained; tr.app rejects because its
  declared code range exceeds the file size.
  These calls are diagnostics, not an application launch or playability claim.
* The optional SD report uses the same image_probe.cpp on host and ARM.
  The owner's private data ZIP was unpacked locally and exercised through that
  path. It contains their firmware/game and is never a public release asset.
* JAR/3DSX/SMDH installer regression covers actual embedded files, replacement,
  identical copies and rejection preserving previous destination data.
* devkitARM builds both the separate probe and main CIA/3DSX. Final CIA is title
  000400000F3A3D00, version 60 / 0.3.12. CTRTool content hashes and extracted
  RomFS/banner/icon/audio PCM comparisons pass; the three expected unsigned
  signature failures remain. Embedded-asset and banner-format checks pass.

## Previous validation history

# Java3DS 0.3.11 validation — 2026-11-05

Physical Java game/lifecycle validation remains required. Current evidence is in
validation/0.3.11; do not interpret host passes as console/game certification.

* Final CIA is title 000400000F3A3D00, version 59 / 0.3.11. CTRTool verifies
  content hashes; the three expected unsigned signature failures remain.
  Extracted RomFS, banner, icon and audio PCM match their build inputs, including
  the new standalone CPU probe. Embedded-asset and banner-format checks pass.
* Real VM built with POSIX threading and a host adapter returning Nintendo's
  1900-based milliseconds passes 22 probes twice. ClockEpochCheck compares Java
  seconds to the independent Windows clock and verifies a bounded Object.wait.
  RecursiveMonitorCheck also exercises mutex/condition deadlines. This covers
  the actual currentTimeMillis conversion used in the console source path.
* Same VM passes live suspend/resume and cancellation while suspended. Five real
  games each survive three cancel/restart cycles, with complete stage-72 cleanup.
* Four targeted 60s host probes exit cleanly. Screenshots show Gladiator combat,
  LOTR gameplay, Orcs II gameplay and Resident Evil's item-selection screen.
  A separate interactive Resident probe passed the reported mission selection.
* Both supplied Orcs II filenames are byte-identical (SHA-256
  2aa27f4082949d6ba59c6a28ffe1f5255e93281a477901994ab062d568a4991e).
* The new supplied device logs have been preserved. They do not capture the four
  current reported failure points; Stack Attack's Siemens API errors/NPE remain
  visible, while the last recorded Orcs II run ends with stage 72.
* The real EKA2L1 dyncom subset compiles and links for ARMv6K/devkitARM. Its host
  test passes ARM, Thumb, memory, SVC, exclusives, VFP32/VFP64, context restore and
  bounded instruction slices. Cache is 8 MiB. The owner supplied a physical New
  3DS probe result: all six checks PASS, zero recorded faults, one million guest
  instructions in 117078 us (8.54 M instr/s). This is a hot arithmetic/branch
  loop, not a game/OS performance test. Old 3DS remains untested.
  All new CPU/probe objects have ARMv6K attributes. The final ELF inherits a v7
  attribute from the SDK's libgcc _sync_dmb.o, as does the existing Java app;
  no DMB/DSB/ISB instruction from that member remains in the linked CPU probe.
* Touch mapper tests pass deadzone, diagonal motion, action-button priority,
  release/restoration, cancellation, portrait/landscape and coordinate bounds.
  One pointer is modeled; no game-specific touch profile is certified.
* No Symbian kernel, phone OS, SIS/game execution or graphics/audio backend is
  ported in this build. The probe's instruction fixture is original project data.

## Previous 0.3.10 validation

Test build; physical 3DS HOME/lid, self-recovery, keyboard and CIA self-update
still require console verification. These limits must not be inferred away from
successful host results.

* CIA and 3DSX compile against the rebuilt modified ARM VM. CTRTool confirms
  title 000400000F3A3D00, version 58 / 0.3.10 and all content hashes. Three
  expected unsigned signature failures remain. Exact extracted RomFS, banner,
  icon and original audio PCM comparisons pass; banner format regression passes.

* 21 runtime probes each pass twice with both Win32 and POSIX threading. New
  probes cover recursive monitor ownership/timed waits, Cyrillic/Greek/Latin
  pixels, both Vir2L menu layouts, name editing and Micro3D primitives.
* Both host threading modes pause/resume a live VM through the platform suspend
  flag; cancellation while suspended also finishes. Five real games each cancel
  and restart three times per threading mode, including complete stage-72 cleanup.
  Request at 3000 ms; three-run totals 9521–9739 ms. This measures PC behavior.
* Interactive DawnStar and Elder Scrolls Travels probes create/name a character
  and enter the actual dungeon. Screenshots/logs in validation/0.3.10. The host
  keyboard bridge supplies Player; the real 3DS keyboard is compiled, not tested.
* Gladiator 3D reaches combat; two previously unprofiled LOTR variants reach
  gameplay in 240x320 portrait. Devil May Cry completes its 60s host run without
  the reported missing renderPrimitives method. These are short checks.
* Nested HOME/lid reason tracking and audio suspend/resume preserve menu pause
  in service-stub tests. Audio failure/Old/New/core-fallback checks pass.
* Production updater logic reaches real GitHub HTTPS without a token and reports
  the private channel correctly. Metadata parsing, hashes, install/backup and
  rejection checks pass; public-window download evidence is recorded separately.
* Source inspection of libctru identified pthread priority 0x3F versus old VM
  worker 0x30, with APT event thread 0x31. VM/Java/GC now use 0x34; contended
  spin loops sleep positively so lower-priority owners can run. Recursive
  libctru condition waits require depth one; Object.wait now releases all levels.
* Symbian audit includes added N-Gage archives and bounded SISX controllers;
  no ROM/RPKG/VPL was supplied. No Symbian execution backend is implemented.

## Previous 0.3.9 validation

PC and file validation completed; physical 3DS results remain unverified.

- CIA/3DSX compile with ARM runtime rebuilt from the modified VM. Same title
  000400000F3A3D00, title version 57. CTRTool container/hash checks and exact
  extracted RomFS/icon/banner/BCWAV comparisons pass. Expected unsigned
  ticket/TMD/access-descriptor signature failures are not content hash failures.
- Sixteen runtime probes each pass twice in one host process, including new
  Choice/TextField/Siemens form/softkey and reflection-based column-depth checks.
- Seven real-game one-shot forced cancellations: 1152–1295 ms total with the
  request at 1000 ms. Four games each pass three forced cancel/restart runs in
  one process (3418–3746 ms total). These are PC latencies, not device timings.
  Catalogue cancellation removes the partial archive. MIDI seek cancellation
  stops after two 256-sample blocks and then permits recovery.
- Micro3D before/after extracted C pixel loops: 10,000 triangles, 40,960,000
  compared pixels, zero differences; 121 ms versus 106 ms host CPU. Full-game
  and physical 3DS FPS still require measurement.
- Soft audio output gain: 1 kHz 0.9997, 9 kHz 0.0735; Warm 9 kHz 0.0279.
  Original bypass is exact. Silence/reset, mixed background/effects, mute,
  pause, core-2/Old/fallback and device-failure stub checks pass. Tiny filter
  state is flushed before denormal arithmetic can stall silent buffers.
- Settings round-trip/migration preserves sound tone, phone orientation and
  Fit (including portrait). Catalogue JAR/JAD, resource preservation, ZIP
  content detection and preparation cancellation pass. Actual C++ menu preview
  was visually checked; nine game menu rows fit the lower display.
- Real host probes: Elder Scrolls Travels completes dungeon/image loading;
  DawnStar passes its prior allocation error; Harem reaches the first level;
  LOTR portrait no longer shows the mode warning; Stack reaches menu/help.
  These are startup/short interaction results, not game completion.
- Orcs & Elves 2 and Doom II RPG each complete a 45s host run and emit actual
  wall-column depth (320×188 and 320×164). 1050 and 415 frames contain positive
  model depth respectively. HUD/menu depth is separate; floor/objects are
  approximate. No claim of generic support for every Doom-engine JAR.
- Treasure's 3D menu renders with MIDI when the proper right softkey is used.
  Reported in-game black world is still unverified/unfixed. Spore Creatures
  lacks its JAR, the supplied NFS Run requires custom native registration code,
  and all reported slow games still need console speed/compatibility retests.
- Banner uses the Faedrops 0.27 mesh-facing layout, with no timed counter-spin.
  276536-byte CGFX, 87 dictionary lookups and 739673 serialized vertex samples
  pass; max authored position error 0.004231 scene units. Four authored rotation
  channels are retained in file data. HOME's actual interpretation/rocking
  still needs a physical test. The Blender original was not edited.

Evidence: validation/0.3.9. Baseline failures and optional missing-API preload
warnings are retained to distinguish earlier errors from the final probes.
The referenced emulators are source/behavior references; Android or Symbian
emulators have not been embedded into the 3DS application.

## Earlier validation (0.3.8)


PC checks completed; physical-console verification remains pending:

- CIA/3DSX build, same title 000400000F3A3D00, version 56 (0.3.8).
  Container, extracted assets and local QR checks are in validation/0.3.8.
- All runtime checks pass twice, including GameCanvas partial/hidden flush,
  rendering, Micro3D, M3G, media callbacks, controls and stop. Stereo integration
  records nine frames with real depth, including the direct buffer flush.
- 1,000 opaque 320x240 flushes, three host runs per variant. Stereo off:
  0.3.7 150/136/147 ms; 0.3.8 42/29/28 ms. Stereo instrumentation:
  845/730/848 ms vs 243/333/280 ms. Both check resulting color pixels.
  These are buffer-transfer timings, not whole-game or physical 3DS FPS.
- Six 35-second scripted probes exit 0: Blades 320x240, Soul, Moorhuhn,
  Yeti, Orcs & Elves II and Witcher. Blades starts MIDI and produces PCM.
- Audio device stubs cover New-core-2 success, denied-core fallback, Old
  fallback, thread/allocation/DSP failures, mixing, pause and shutdown.
  CIA requests CanAccessCore2. Device scheduling remains untested.
- Seven CRT modes preserve settings. LUT output equals reference math over
  all phases/channel values; alpha/black and dark contrast checks pass.
  Preview uses the actual C++ filter. UI remains unfiltered.
- Banner: 264216 bytes, 86 dictionary lookups. Four original source tracks
  repeat every five seconds. A separate root channel counters an assumed
  eight-second external orbit, combined period 40 seconds. Serialized paths
  and matrices match GLB at 640 samples within 0.00004203. This assumes the
  user's approximate orbit estimate, not verified HOME Menu timing.

**Banner is still experimental.** A timed inverse orbit is not a universal
rotation-disable flag. Different speeds or microphone/motion response can
cause drift. Console playback over several cycles is required before calling
this fixed. The original source motion/artwork and sound are retained.

## Previous 0.3.7 validation — 2026-11-02

Completed on PC; physical-console checks remain pending:

- CIA/3DSX build, same title 000400000F3A3D00, CIA version 55 (0.3.7).
  CTRTool verifies container/content/ExeFS/RomFS hashes. Ten RomFS files,
  icon, banner, decompressed CGFX and embedded WAV samples equal the sources.
  Three unsigned-homebrew signature failures are expected.
- Two VM runs per runtime regression: rendering, sensors, M3G asset/spans,
  Micro3D poses/batching/blending, arcs, GameCanvas flush, media callbacks,
  RMS, controls, DirectGraphics and shutdown reentry; all return exit 0.
- Micro3D opaque batch output remains pixel-identical to the per-triangle
  reference. New blending checks cover half/add/subtract, saturation,
  color-key, black additive identity, disabled transparency, no translucent
  depth writes and opaque-first batch submission.
- Arc checks cover quarter/negative/zero angles, clipping and a five-degree
  wedge matching Yeti's golf-club draw call. GameCanvas checks demonstrate
  no second paint, preserved buffer/partial flush and no hidden-canvas output.
- Eighteen AMR clips from the user's Blades build match direct decoder PCM
  exactly at 8 kHz. Duration, EOF, truncated/invalid AMR and no SoundFont load
  pass. Existing MIDI/WAV lifecycle, lazy bank and independent mixer checks pass.
  Signed PCM interpolation was corrected after this comparison exposed wrapping.
  Soul bytecode confirms silent effect ID -1 and loop -1 converted to one play.
- Six real games run 35 seconds with exit 0: Blades 320x240, Soul of Darkness,
  Moorhuhn DeluXe, Yeti Summer, Orcs & Elves II and Witcher. Captured Moorhuhn
  has a menu, chickens and crosshair, score changes after shooting. Orcs
  reaches the dungeon. These timed scripted runs are not full-game coverage.
- Fresh-save Blades with sound Yes loads/starts two MIDI assets and produces
  nonzero PCM (peak 10195). Earlier sound-No scripted runs intentionally
  produce no music. Host frame counts are not measurements of device FPS.
- Witcher and Soul frames contain distinct positive/negative depth with Auto,
  rather than HUD-only metadata. Layer estimates are heuristic. Orcs uses its
  own Java renderer then drawRGB; it has no Micro3D/M3G depth integration.
- Auto settings migration, new explicit Off persistence, CRT filter round-trip,
  scanline/grid alpha preservation, compositor and HUD tests pass. Eight-row
  menu/loading previews are included in validation/0.3.7.
- Banner is 264072 bytes, all 85 dictionary lookups pass, four animation
  members and a five-second loop remain. Rotation curves span at most about
  64 degrees, not a full turn; text has translation channels only. Static
  Facing anchor is YAxial to counter HOME Menu whole-model rotation. Badge
  removed. Sound normalized by 32.99 dB to -1 dBFS, stereo PCM16/32000;
  embedded samples equal the processed WAV. Desktop originals are untouched.

Still to validate on 3DS: HOME Menu motion/materials/audio timing; Old/New
memory headroom, 3D speed and sound; comfort of 2D layer hints; reported menu
flicker and loading transitions. Software rasterization, per-mesh translucent
ordering and incomplete custom-renderer depth remain limitations.

## Previous 0.3.6 validation — 2026-11-01

- Nokia drawPixels direct path checks reverse scan, clip and alpha against
  the prior image/region route (alpha rounding can differ by one channel level).
- Micro3D native batch versus per-triangle reference: exact pixel comparisons
  for perspective/parallel projection, near clipping, texture alpha, flat color,
  clips and pattern masks. Invalid indices and NaN inputs remain bounded.
- Blades 320x240, same host/config/input script, 35 seconds each, batch disabled
  then enabled: 602 / 726 published frames, process CPU 21750 / 17406 ms,
  wall time 35.711 / 35.245 s. Both exit 0 and render the forest/tutorial.
  This is not a 3DS FPS claim. The synthetic gather/submit test is separate.
- Stereo compositor unit checks flat identity, opposite eye shifts, gap filling,
  model/layer depth selection, stationary HUD priority and release after stop.
- Blades and Outland capture positive/negative depth in real gameplay frames,
  with later HUD tags preserved, and stop with exit 0. Captures use a rendered
  scene frame; the shutdown repaint can legitimately be flat.
- VM integration captures depth from LayerManager, Micro3D and M3G direct-canvas
  paths. Colors use the existing rasterizer; stereo only changes presentation.
  Mutable-image/clone/region/getRGB propagation is exercised separately.
  Reconstruction of hidden geometry and arbitrary array-copy tracking are absent.
- Settings preserve older files and round-trip stereo choices/strength; invalid
  values fall back to defaults. UI preview includes the seven-row game menu.
- Banner: 502712 bytes, 105 serialized dictionary lookups pass, four animated
  members, same five-second loop. A static BUILD 0.3.6 marker distinguishes it
  from earlier assets. Supplied PCM16 WAV retained. The Desktop source is intact.
- Physical HOME Menu cache/framing, stereo comfort, speed and Old/New 3DS output
  still require a console test. PC validation cannot establish those results.

## Previous 0.3.5 validation — 2026-11-01

- Dumps 29/30: process `menu`, identical PC 0x187d58 / LR 0x20b940 /
  FAR 0x8, before application startup. No HOME Menu symbols are available;
  application ELF addresses must not be used to symbolicate this crash.
- Reproduced invalid Patricia lookup in the actual 0.3.4 CGFX at 0x2ac4:
  `Scene root` resolves to `GreasePencil.002`. All five skeleton entries fail.
  Upstream opacity sorting reorders nodes without regenerating branch indices.
- Conversion rebuilds that tree and sets rigid SOBJ mesh node names. All 83
  serialized lookups in 13 dictionaries now pass. Regression rejects reordered
  entries with otherwise valid pointers and rejects truncated files. The old
  0.3.4 binary is rejected by the same build gate.
- Geometry, fixed camera, 300-tick loop and glass materials are retained.
  New original WAV is stereo PCM16/48000, 88616 frames (1.846167 seconds).
- CIA/3DSX build, title version 53, same title ID. Container hash and embedded
  resource checks are recorded in validation/0.3.5 after packaging.
- Physical HOME Menu highlighting, audio, launch, exit and return to icon are
  pending a console retest. Format and container checks do not replace it.

The following records describe the earlier 0.3.4 runtime validation. Its
banner passed container hashes but was subsequently shown to crash HOME Menu;
those earlier checks did not validate Patricia lookup.

## Previous 0.3.4 validation — 2026-11-01

PC validation, with physical-console checks still pending:

- CIA/3DSX compile; CIA version 52 (0.3.4), same title 000400000F3A3D00.
  CTRTool verifies content/ExeFS/RomFS hashes. All ten RomFS assets, icon and
  banner compare to source. Unsigned-homebrew signature failures are expected.
- M3G differential test: 90 scenes (perspective, vertex colors, textures,
  alpha/depth, all five common compositing modes and both fog modes). Native
  matches Java except twelve exponential-fog pixels with 1-channel-level
  rounding (2 after MODULATE_X2). Example PC timing: 641ms versus 146ms.
- Micro3D pose/transform regression covers the model-buffer sentinel tail,
  matrix result and invalid bone counts. Blades 320×240 and 240×320 each reach
  rendered forest/tutorial scenes in 45-second PC runs, exit 0. The new dump 28
  matches a separate metadata-allocation failure; console retest remains needed.
- Outland: real cave scene, 30-second run, 304 published frames; 73,570 native
  spans / 701,547 native pixels / zero fallback pixels, exit 0.
- Orcs & Elves II: final 25-second startup run exits 0; an earlier 35-second
  iteration reached the dungeon/tutorial and played MIDI. Worms: gameplay scene, 45 seconds, 712
  frames, complete bodies, MIDI/WAV PCM, exit 0; console load issue pending.
  Final Blades 320×240 repeat after memory changes: 40 seconds, 645 frames, exit 0.
- Two repeated runtime regressions cover rendering, M3G, sensor connections,
  RMS, controls (including touch softkeys), DirectGraphics and shutdown reentry.
- Pause test freezes published frames, resumes them, and exits normally for two
  VM runs. Cancellation while paused returns internal status 2 and destroys VM
  cleanly. Backlight state test covers timer, first-touch consumption, held touch,
  always-on setting, start-asleep and menu keep-awake.
- Catalog regressions include real ZIP named .jad, .jar.jad descriptor sibling,
  missing binary, JAR preservation, resource rewriting and LGE metadata.
- Updater and CIA heap/stream/target/integrity regression checks pass.
- UI functions render library, depressed keypad softkeys and game menu on PC.
  Three supplied WAVs converted to mono PCM16/22050; actual DSP channel-1 UI
  output and coexistence with game audio need console testing.
- Source camera matrices are identical over all 120 original frames. Banner
  source is a compact derivative; the Desktop original was not changed. Four
  model animation members, 300 ticks/5s loop, 484616-byte CGFX; Blender preview
  is approximate and does not prove HOME Menu framing/material appearance.

- Dump 28 matched against saved 0.3.3 ELF and instruction bytes: NULL UTF-8
  metadata allocation. Three parser fault tests cover NULL string/record and
  failed growth; a real Class.forName fault test reports a VM error and reaches
  destruction without a native crash. This is targeted, not universal OOM handling.
- SoundFont now loads on first MIDI start, with shared lazy bank lifetime and
  PCM16 sample storage. Eleven rendered asset hashes are bit-identical to the
  previous float bank. WAV and MIDI parsing leave the large bank unloaded.
  Tracked audio allocations: 5292-byte idle for two engines, largest allocation
  16028374 bytes, peak 18392672 bytes. Closing restores the process-start allocation
  baseline. All 128 GM melodic programs and percussion produce nonzero PCM.

New evidence: validation/0.3.4. Hardware checks: Blades crash retest, 3D speed,
Worms loading, menu pause/exit, backlight touch wake, UI audio and HOME banner.
Spore needs the actual missing JAR. No claim of all games being playable.

## Historical 0.3.3 validation

# Java3DS 0.3.3 validation — 2026-11-01

All tests below ran on the Windows PC. No physical console was controlled.

- Age III: two consecutive 12-second final-host runs render frames and return 0.
- Supplied dump 26 symbolicated with the preserved 0.3.2 ELF: failed 19200-int
  allocation in jarray_create_by_class, followed by a NULL write. Deterministic
  calloc fault injection for that allocation: two runs caught OutOfMemoryError
  and returned status 0. No claim that every hardware OOM path is eliminated.
- Java adapter, optimized ARM runtime, 3DSX and CIA compile. CIA title version
  51 (0.3.3), title ID 000400000F3A3D00. CTRTool validates content, ExeFS/RomFS
  hashes and extracted files. All seven RomFS files match source; banner matches
  the prior asset. Three unsigned-homebrew signature failures are expected.
- Two consecutive VM runs each pass RGB negative stride, native font clipping,
  4096-pixel sprite decode/region draw, software depth, overflow allocation,
  deferred self-rescheduling callbacks, M3G triangle drawing, real M3G final-file
  section loading, sensor discovery/Connector rewriting/buffers/close, RMS
  persistence/names/enumeration, DirectGraphics, controls and shutdown reentry.
- Sensor arithmetic verifies 512 counts/g and gyro counts-per-degree conversion.
  Host motion samples are explicit fixtures; console axes/calibration need testing.
- Original eleven MIDI/WAV fixtures pass render/lifecycle checks. Two MIDI players
  plus one WAV pass independent start/stop/seek/close. Simulated DSP/cache/allocation/
  thread failures and audio shutdown pass. This does not test physical speakers.
- Identical-image RGB/font PC microbenchmark: 393ms prior path versus 10ms new path,
  hash 427041193. Not a whole-game or 3DS speed measurement.
- Real Blades and Magic: forest, player and boar visible, 60-second host run exits 0.
  Real Outland: cave rendered, 25-second run exits 0; root Group/Mesh asset verified.
  Gladiator: skinned models/animations load, 45-second run exits 0.
- Discworld: intro dialogue reached (previously reported infinite loading).
  Worms: 45-second run reaches gameplay with complete worm bodies, exits 0,
  MIDI/WAV players load and produce PCM. These are sampled scenes, not full levels.
- Catalog checks pass JAR/JAD, URL decoding, multi-MIDlet, invalid input, original
  JAR preservation, resource/Connector rewriting and LGE 240×320 resolution.
- Updater version/manifest/hash/length/target/short-write/cancel and bounded CIA
  heap checks pass. Actual private Wi-Fi and CIA replacement need a console test.

- All 65 supplied JARs completed a 7-second PC startup/cancellation run with
  process status 0. Thirteen titles logged missing API references; seven
  logged Java exceptions/errors (some caught by the game). Full report: validation/0.3.3/compatibility.md.

The batch test is a timed startup/cancellation smoke check. Returning status 0
is not proof of gameplay: missing optional methods and caught Java exceptions
can still be logged. See the accompanying compatibility report for each title.
Remaining gaps include TextBox/Alert, FileConnection, Bluetooth, MIDIControl,
compressed sound codecs and incomplete 3D effects. Both 3D backends are software
and experimental. Banner framing is deferred as requested.

Reproduce portable checks after build-java.ps1 / build-host.sh / tests/build-probe.sh:
`py tools/runtime_checks.py --java-home PATH_TO_JDK --host work/java-host.exe`.
Optional `--m3g-jar PATH_TO_OUTLAND_JAR` checks the real prize.m3g section regression
without redistributing that game asset. Fault injection uses the separate
`JAVA_ALLOC_FAILURE_TEST=1` host build and JAVA_TEST_FAIL_19200=1.

Physical retest priorities: Age III memory/load and exit; LocoRoco resolution/
speed; Batman audio/speed; Worms bodies; Discworld progression; experimental
Blades/Outland/Gladiator; Old 3DS RAM, touch-only feedback, parallax and Wi-Fi.

## Historical 0.3.2 validation

# Java3DS 0.3.2 stability validation — 2026-11-01

- CIA/3DSX compile; CIA TMD version 50 (0.3.2), same title ID.
- CIA contents, ExeFS/RomFS hashes and extracted resources checked with CTRTool.
  Homebrew ticket/TMD/access-descriptor signature failures are expected.
- Age of Heroes III reproduced a host access violation in array_class_create_get
  for missing com/nokia/mid/sound/Sound. After the API and VM fix, two 12-second
  runs render frames, play MIDI (PCM peak 28623), and return status 0.
- Age V/VII, LocoRoco Hi, Dark Knight Rises: two 12-second host runs each render
  frames and return status 0. This covers smoke startup and cooperative stop,
  not all gameplay or the physical console crash reported by the user.
- Missing array component regression: three runs report ClassNotFoundException
  without a native crash. RMS: three runs verify names, Unicode, path safety,
  zero-length null data, persistence, listing and deletion.
- Shutdown callback reentry: three VM runs complete without the old class-monitor
  deadlock. Updater target/size/hash/title-ID/stream short-write tests pass.
- CIA heap tests cover 64/124 MiB profiles, constrained resource limit and fallback.
  Final ELF startup call is linked through the bounded-heap wrapper.
- Physical CIA startup, Dark Knight first load, START+SELECT and game save UI
  need a console retest. 3D backends are still unsupported. Banner untouched
  during this stability pass; its motion/framing needs discussion with the user.

Previous 0.3.0 validation follows (historical):

# Java3DS 0.3.0 validation — 2026-10-28

This build was compiled and tested on the Windows PC. The user's previous
hardware reports concern earlier builds; 0.3.0 still needs a physical 3DS test.

* JDK 17 / Java 8 bytecode adapter and demo compiled. devkitARM runtime,
  native launcher, libcurl/mbedTLS updater linked into the final 3DSX.
* All seven RomFS files extracted from the final 3DSX match their source bytes:
  bootstrap, MIDP, demo JAR, GM SoundFont, private PoP profile/theme, CA bundle.
* Catalog regression: JAR/JAD, local JAR resolution, URL-encoded names,
  continued manifests, multiple MIDlets, invalid files, source preservation,
  resource bytecode redirection and known PoP replacement pass.
* Launcher Check: two consecutive VM runs, 61 frames each, status 0,
  embedded resources, persisted RMS counter and MIDI PCM peak 9244.
* PoP 400×240 profile: two consecutive VM runs reached gameplay state 10,
  726 / 725 frames, status 0. This test does not establish full gameplay or
  physical camera/input correctness. Its scripted audio peak was zero.
* Might and Magic II [320x240] E62/E71: no unresolved native/API methods in
  the tested path after adding DirectGraphics. Reached the animated intro
  and subsequent character/stat screen, 644 frames over the 50-second test,
  status 0, MIDI PCM peak 31469. Full-level play is not yet verified.
* Soul of Darkness [320x240] SGH-i617: reached an in-game dialogue scene,
  status 0, MIDI PCM peak 15076. Inspected frame had no duplicated lower strip.
  Optional Form/StringItem API references are still unresolved, though they
  were not invoked by this tested path. This is not complete MIDP support.
* DirectGraphics fixture passes RGB565 roundtrip, ARGB8888, counterclockwise
  90-degree rotation and horizontal flip. Controls fixture passes custom
  remapping, PoP B+direction, A action and diagonal stick without jump.
* DaemonExit: 12 consecutive VM launches and clean exits in one process.
* Cancellation fixtures: infinite Java loop holding a monitor with another
  thread blocked on it; Object.wait(0); Thread.sleep(1 hour). Stop requested
  after 1.8 s; all returned and reached VM destruction. Loop cancellation
  returns internal status 2, which the launcher suppresses for requested exits.
* Updater: semantic version selection, release metadata and URL checks,
  SHA-256, length mismatch, tampered payload, wrong file magic, successful
  replacement and previous-version backup tested using local files.
  `bash tests/build-updater-check.sh` reproduces the portable updater checks.
* HTTPS CA bundle verified against the live public GitHub API on PC, HTTP 200.
  This does not validate 3DS networking or the user's private token.
* Actual UI drawing functions rendered to host images and inspected:
  library, game settings, bindings, depressed keypad key, loading and updates.

Retained evidence in the private source ZIP: work/*-v03.log, work/ui-v03.png.
The full MIDI bank and audio device engine were unchanged; previous tests of
all 128 GM melodic programs, percussion, background music, WAV mixing and
simulated DSP failures remain in gm-test.log and device-test.log.

Hardware checks still required: absence of top-screen flicker/strip artifacts,
Might and Magic II gameplay, touch alignment and pressed feedback, game exit,
actual DSP output, Old 3DS memory/performance, private Wi-Fi check/download and
SD replacement. Blades and Magic Micro3D is explicitly unsupported.
