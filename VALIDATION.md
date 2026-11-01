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
