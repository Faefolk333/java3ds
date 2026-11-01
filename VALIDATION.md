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
