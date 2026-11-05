# Symbian implementation — CPU/EKA1 loader foundation, no game runtime yet

The main Java3DS app executes Java ME bytecode. It does not yet execute Symbian
SIS/SISX/APP programs. The inventory in validation/0.3.10 was metadata-only.

## Confirmed startup/E32 checks and loading progress (0.3.14)

The owner supplied 0.3.13 New 3DS logs: startup and every CPU/E32 fixture check
pass, with zero CPU faults (1M hot-loop instructions in 114821 us). The supplied
Shadowkey image validates/maps on hardware. The owner confirmed pressing START
during ROM loading. Thus full ROM reading/linking is not yet console-verified.
The old diagnostic erroneously proceeded to entry/export calls after that
cancellation; its EUSER#1582 stop is not evidence of a ROM/library failure.

A task-wide latched control now stops I/O/linking/execution after any false poll.
An original test ROM reproduces one false poll followed by true polls; no guest
call or completion record may follow the cancellation. ROM mappings roll back.
64 KiB reads emit byte progress; the probe shows percent, KiB and elapsed time.
B cancels only the image diagnostic; Y explicitly starts a fresh task. START
exits the application. Cancellation logs include stage/bytes/elapsed and the
platform input source. New controls and read throughput require a device test.

## Standalone startup correction (0.3.13)

The owner's dump 31 matches the 0.3.12 probe startup, before main. A 124 MiB HBL
process with the fixed 2 MiB linear reservation caused the SDK to request
121.59 MiB of regular heap, beyond its 96 MiB address range. The existing CIA
bounded-heap hook is now linked into the probe and main 3DSX as well. Static
ELF call-target checks and a regression using the dump's inputs cover the fix;
the owner subsequently confirmed startup/all fixture checks passed. The earlier private 0.3.12 SD data ZIP works
unchanged. This does not add a kernel/heap service to the emulated Symbian OS.

## EKA1 E32/ROM stage (0.3.12)

The standalone probe now loads uncompressed ARM EKA1 E32 images, validates
checked UIDs and all parsed ranges/counts, maps code RX and data/BSS RW, applies
text/data/inferred relocations and handles EKA1 code-relative exports. Guest
addresses use checked region lookups with a 24 MiB budget, not host pointers.
The initial loader limits files to 8 MiB, imports/exports to 8192 each and
relocations to 131072, bounding metadata as well as mapped memory.
Calls have bounded instruction slices and cooperative cancellation. Unsupported
imports/SVCs report their exact identity; memory violations stop inside the guest.

An uncompressed EKA1 SYM.ROM (up to 20 MiB, base 0x50000000) can be streamed into
guest RX memory. A ROM DLL's first 100 bytes must match the corresponding bytes
in the mounted ROM, including its UID. Its requested exports are checked before
IAT changes. This avoids combining mismatched device dumps. ROM reads pump the
platform callback every 64 KiB. EKA2, compressed E32/ROM and SIS installation are
not supported in this stage.

Host evidence: supplied Shadowkey has 1,051,076 bytes of code, 415 imports and
10,104 relocations. Its 18,284,544-byte RH-29 ROM resolves 324 imports from 18
libraries; GAMECOMMS and SIMKIN remain unresolved. DLL entry returns, while its
first export enters real EUSER code and stops on EKA1 SVC 0x6c (Heap). Upstream
svc.cpp maps that SVC to the current thread's heap allocator. No allocator,
thread/kernel services or gameplay is provided yet. The new loader/ROM path
needs a hardware test; 0.3.11's hardware pass covered only the small CPU fixture.

For the optional SD diagnostic, put test.app, SYM.ROM and matching 100-byte ROM
DLL descriptors under /3ds/java-3ds/symbian/rom-libs. The two main files belong
in /3ds/java-3ds/symbian itself. Press Y in the separate probe; collect
/3ds/java-3ds/symbian-loader.log. tools/prepare_symbian_test.py prepares this
private SD ZIP from owner-supplied paths. Firmware/game inputs are never staged,
packaged with Java3DS, or uploaded. Test fixtures in source are original code.

Host reproduction: py tools/build_symbian_loader.py. ARM build:
py tools/build_symbian_probe.py. The CPU library remains separate from the Java
process. Remaining work is the actual EKA1 kernel/thread/heap environment,
recursive game DLL loading, services/graphics/audio and later EKA2/SIS support.

## First implemented component (0.3.11)

The separate `java3ds-symbian-probe.3dsx` contains the real EKA2L1 dyncom CPU
interpreter adapted to ARM11/devkitARM. Its translation cache is bounded to
8 MiB; the upstream 125 MiB default exceeds the console budget. Newlib integer
type mismatches in decoder/VFP signatures are corrected. A host integration
probe executes ARM and Thumb instructions, memory loads/stores, SVC callbacks,
exclusive operations, VFP32/VFP64, context restoration and bounded time slices.
The owner ran this probe on New 3DS on 2026-11-05. ARM, Thumb, exclusives,
VFP, context restoration and bounded slices all report PASS, with zero recorded
faults. Its one-million-instruction arithmetic/branch loop took 117078 us
(8.54 million guest instructions/s). This validates these small fixtures on
hardware; it is not a game-speed estimate or exhaustive CPU conformance test.
Evidence: validation/0.3.11/device-symbian-probe.log and .json. Old 3DS is untested.

Run the separate probe through Homebrew Launcher. It records the CPU results
and benchmark in `/3ds/java-3ds/symbian-probe.log`. The main CIA/3DSX launcher
extracts it to `/3ds/java3ds-symbian-probe` at startup in the corrected source.
The published 0.3.11 build incorrectly applies JAR validation to the probe and
leaves that folder empty: manually download the release's standalone 3DSX into
that folder and launch it in Homebrew Launcher. The fix is on main for the next
build, with real JAR/3DSX/SMDH copy and rejection regression checks.
The lower screen previews
Circle Pad/D-pad movement as virtual joystick touches, A/B as virtual action
touches, and direct touchscreen input. X toggles portrait/landscape coordinates.
The mapper has configurable normalized anchors. It models one pointer: action
touches take priority, releasing then restoring joystick movement. Simultaneous
multi-touch gameplay and individual game's exact control positions are not
implemented or certified. The mapper is not connected to a running Symbian game.

The probe runs only original small instruction fixtures; no phone OS, game or
firmware is embedded. There is no SIS launcher pretending to execute games.
Remaining work includes EKA1/EKA2 kernel and services, completion of the memory/ROM/E32
loaders, package installation, filesystem, fonts, graphics/audio and main-app
integration. First measure the CPU component on Old/New hardware before choosing
the system memory/cache budgets. Source/licensing: third-party/eka2l1-cpu/README.md.

* Shadowkey and Tomb Raider include EKA1 / ARMv4 E32 applications for N-Gage.
* Asphalt Urban GT 2, Call of Duty, Splinter Cell: Chaos Theory, Tony Hawk's
  Pro Skater and Worms World Party archives contain native `system/apps/*.app`
  files, rather than JARs. Several supplied archives are demos or review builds.
* Hero of Sparta and Prince of Persia HD contain native S60v3-or-later SISX
  executables. Their controller metadata names EXE payloads, not embedded JARs.
* At the 0.3.10 release audit, no device ROM, RPKG or VPL was supplied.

## Later supplied N71 firmware — 2026-11-05

The owner subsequently supplied Nokia N71-1 / RM-67 firmware 4.0642.1.05,
product 0526140. The local `system/Firmware` package contains a VPL, MCU FPSX,
PPM V02, DCP and signature, plus an additional V05 language image. Every file
referenced by the VPL exists. CRC32 matches both declared images exactly:
MCU 69750F87, PPM V02 25500325. Metadata identifies the Turkey / EURO 2 variant.
The VPL references V02; the extra V05 is not selected by that manifest.

This removes the absence of firmware for this particular device. A subsequent
manual Windows import hit `Unrecognised block type 147` in the FPSX reader;
successful N71 installation is not established. Local audit:
`work/symbian-n71-audit.json`.
Firmware and derived device files stay outside Git and release archives.
The 3DS execution backend is still not implemented.

## N-Gage system imported successfully — 2026-11-05

The owner supplied original N-Gage, QD and QD Dev system data and manually
imported a SYM.ROM successfully into the isolated Windows test application.
Its `data/devices.yml` now records RH-29 (N-Gage QD), version V 04.10, EPOC6,
machine UID 270512817. The log completes app-registry loading. This confirms
device import. The owner subsequently confirmed Shadowkey runs smoothly in PC
EKA2L1. RM-320 (N95 8GB) was also imported and the owner reports SIS installation
and execution succeeded. Those are PC reference results, not 3DS results.

EKA2L1 needs a compatible device/OS image in addition to the game:
https://eka2l1.github.io/quickstart/basic/installdevice/
Firmware is the phone's operating system; a game APP/SIS does not replace it.
No firmware is bundled or downloaded by Java3DS.

Upstream https://github.com/EKA2L1/EKA2L1 has no Nintendo 3DS target. A port
needs platform/kernel integration, a CPU execution strategy, graphics adaptation
and memory-budget work, followed by device-image and game tests. The DualStick
fork adapts Android input; it supplies no 3DS execution backend. Upstream was
inspected locally. The 0.3.11 separate probe links the CPU subset only; the
full EKA2L1 OS/game execution backend has not been ported.

`tools/symbian_audit.py` reads E32 headers and bounded SIS controller metadata;
optional `py7zr` lists 7z members. Originals remain unchanged. The report makes
no playability claims.
