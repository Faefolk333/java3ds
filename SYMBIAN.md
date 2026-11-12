# Symbian implementation — CPU/EKA1 loader foundation, no game runtime yet

## Native unwind, thread context and ROM entry batch (0.5.0)

The owner's 0.4.1 New 3DS pass validates ROM/cleanup creation. This larger
batch, still needing console validation, adds checked EKA1 76-byte TTrap frame
push/pop services (64 frames maximum), per-thread TLS (64 slots), scheduler
selection and controlled current-thread/process exit. Invalid/duplicate/cyclic
trap chains stop inside the guest. No host CPU context or pointer is restored
from a guest frame: the original ROM instructions perform the saved-register
return through the checked CPU/memory callbacks.

Original ARM diagnostics call UID-checked EUSER Trap/UnTrap/Leave and cleanup
exports. They test normal return, -42 Leave, nested -43/-42 errors, register
restoration, LIFO cleanup and User-allocated pointer cleanup. On PC, 512 further
normal/Leave cycles reuse the same 256-byte native allocation. These fixtures
are original project code and contain no ROM instructions or game payload.

Native CActiveScheduler and CActive constructors/functions are exercised using
bounded diagnostic storage and original GCC2 callback tables. Manually completed
fixture statuses dispatch RunL in priority order, a Leave reaches RunError, the
empty queue returns false and native destructors unregister all objects and
clear the scheduler pointer. This does not implement a complete asynchronous
scheduler, timers or service-server request completion. Native UserSvr DLL TLS
set/get/overwrite/free is also exercised through the ROM wrappers.

Synchronization now supports named mutex/semaphore creation and named semaphore
open in the single modeled process/kernel namespace. Duplicate names return
KErrAlreadyExists. Sixty-four objects and 64 additional alias handles are bounded;
IDs are never reused. Closing an original handle preserves the shared object
while an alias remains; the last close removes the name and backing state.
Process filename/command line and current-object rename use bounded checked
UTF-16 descriptors, including HBufC-backed pointer descriptors. Only current
pseudo-handles are supported. Character case/fold/category operations currently
cover ASCII; non-ASCII calls stop. ASCII fold-to-lowercase semantics are checked
against SymbianSource euser/unicode/unicode.cpp (EFoldCase):
https://github.com/SymbianSource/oss.FCL.sf.os.kernelhwsrv/blob/master/kernel/eka/euser/unicode/unicode.cpp

Read-only ROM path lookup walks only requested directories, with at most 16
roots/components, 2048 entries per directory and 16384 entries total. UTF-16
names compare in ASCII case-insensitive paths; traversals, cycles, malformed
ranges and ambiguous hardware variants are rejected. Serialized directory size
excludes its four-byte length field. A ROM executable's logical file length
need not cover its full resident code; that code is checked against the ROM.

The real AppRun is looked up as Z:/System/Programs/AppRun.exe, with validated
entry layout and already attached ROM dependencies. Its bounded diagnostic
thread receives a virtual test.app command line, not an installed phone/game.
It now reaches FileServer session creation (executor 0x16), which remains
explicitly unsupported. No file service, window/font/graphics/audio servers,
full process scheduler, EKA2/SIS installation or playable frame is claimed.
The same private 0.4.0 SD dataset works; no extra firmware files are needed.
Native-stage cancellation is latched and prevents further app/process calls.

## ROM DLL attachment and native cleanup bootstrap (0.4.1)

The owner's 0.4.0 New 3DS result confirms all import/heap/app UID steps, with
all fixture flags passing (4610 ms total). The next runtime stage follows EKA1
ROM DLL dependency tables directly in the mounted ROM, deduplicates cycles and
prepares a complete dependency-first call list before executing any entry.
It verifies entry-at-code-start headers, dependency table identity on repeated
edges, flag-1 paired entry/table format and all ROM ranges. Bounds: 256 DLLs,
128 references per table and 8192 edges. Unsupported static writable data/BSS
or entry layouts stop explicitly. No backward instruction scanning is used.

The private host dataset completes all 114 ROM DLL thread-attach entries,
then both game DLL entries and the main application DLL entry (reason 1).
The runtime next invokes the matching EUSER ordinal 746, CTrapCleanup::New.
New checked EPOC6 TrapHandler/SetTrapHandler SVCs retain the thread-local guest
pointer and return the previous value. The real ROM constructs its cleanup
object at 0x20000080 and installs its handler at 0x20000084. The application
factory runs afterwards and returns 0x200000ec with the correct UID.

The checked ordinal/name comes from the pinned EKA2L1 epoc6_n.def; native code
and returned pointers are verified against the actual owner's ROM. This is
cleanup/handler creation only: native Leave unwinding, trap frame stacks,
application framework/process environment, scheduling, filesystem/window/audio
servers, EKA2 and SIS installation remain incomplete. No game frame is run.
The owner's 0.4.1 New 3DS logs confirm ROM_INITIALIZATION ready=1 completed=114/114
and NATIVE_CLEANUP ready=1 with the same pointers as the host run. All fixture
flags pass, including trap-handler state, and zero CPU faults are recorded.
The app factory returns 0x200000ec and its UID matches. The complete diagnostic
takes 5589 ms; this is not a gameplay benchmark. Evidence:
validation/0.4.1/device-v041-*.log and hardware-v041-result.json.
Existing private 0.4.0 SD data remains sufficient.

The next error-path work requires checked EKA1 TTrap frame push/pop SVCs
0x72/0x73 (the pinned upstream kernel::trap has 16 saved words and three pointer
words), followed by a native trapped Leave/cleanup-unwind test. These services
are not yet implemented by the current model. Framework/process environment
and real service servers remain further prerequisites for a game window.

## Recursive EKA1 game DLL linking (0.4.0)

Before heap/bootstrap calls, the diagnostic now resolves the complete main/game
DLL graph. Matching ROM descriptors stay in rom-libs; owner-supplied, validated
uncompressed E32 DLLs stay in game-libs with canonical uppercase basenames.
UID-qualified and legacy basename-only imports are supported. DLL code/data/BSS
receive distinct checked mappings and relocations. Dependencies are deduplicated,
cycles terminate and exports are checked before main IAT patches commit.
On error/cancellation every tentative DLL/import mapping is removed; an existing
main image and its previous links remain intact. No guest code runs during linking.
Limits: 32 game DLLs, 64 providers, 8192 total import slots, 131072 cached exports,
24 MiB mapped guest memory. File buffers are freed before recursive descent.

On PC the supplied Shadowkey/RH-29 data now resolves 415/415 main imports plus
284 GAMECOMMS/SIMKIN imports. Following native heap creation, both game DLL
thread-attach entry points return zero in dependency order. The application
entry/factory return, and a checked EKA1 GCC2 primary CApaApplication vtable call
returns AppDllUid 0x101fd3f6. This supports only the primary single-inheritance
GCC2 layout (two-word prefix); it is not a generic EKA2 virtual-call adapter.
The public API declaration order can be inspected at
https://github.com/SymbianSource/oss.API_REF.Public_API/blob/Symbian3/epoc32/include/mw/apaapp.h
and actual EKA1 instructions/return values are verified with the owner's input.

The previous 0.3.15 heap/factory path passed on New 3DS. The owner's 0.4.0 New 3DS logs now confirm the complete import graph, both
game DLL thread-attach entries and matching UID, with all fixture flags passing
and a total diagnostic time of 4610 ms. Evidence: validation/0.4.0/device-v040-*.log
and hardware-v040-result.json. No game frame is run. A new private local SD ZIP adds the two
game DLLs and eight matching ROM headers; tools/prepare_symbian_test.py now walks
dependencies rather than omitting game DLLs. None of those original inputs is
tracked or included in the public release. Collect DEPENDENCY_GRAPH,
DLL_THREAD_ATTACH and APPLICATION_UID records from symbian-loader.log.

This does not complete the application startup sequence: ROM DLL thread
initialization, framework/process environment and service servers remain missing.
PreDocConstructL/document/UI construction must follow that environment setup;
the probe does not skip those steps and claim a running game. Full scheduling,
filesystem/window/audio services, EKA2 and SIS installation remain incomplete.

## Earlier implementation

The main Java3DS app executes Java ME bytecode. It does not yet execute Symbian
SIS/SISX/APP programs. The inventory in validation/0.3.10 was metadata-only.

## Initial EPOC6 executive and native RHeap bootstrap (0.3.15)

The separate runtime now has an opt-in, single-thread EPOC6 executive model.
Supported services cover HAL page size (4096), anonymous normal RChunk creation,
base/size/max queries, grow/shrink, handle close, Heap/HeapSwitch, synchronous
executor request completion, request-count consumption, recursive mutexes,
semaphores and serial locked increment/decrement. Handles are bounded to 64
objects and never reused as live IDs. Chunk virtual holes are reused; the full
reservation is charged to the 24 MiB guest budget, while accesses are restricted
to committed bytes. Decommit/recommit clears contents. Each chunk is limited to
4 MiB. Only one emulated execution thread exists; a would-block operation reports
that scheduler support is required rather than continuing as a completed wait.

EKA1 SVC return is implemented through LR with the Thumb bit restored. Unknown
SVCs/executor operations stop; unsupported known HAL/device modes return errors.
Original ARM/Thumb fixtures and negative tests cover these semantics without a
phone ROM. Guest addresses/descriptors always go through checked memory access.

initializeHeap uses UID-checked, matching EUSER exports 166 and 1127 to execute
real ChunkHeap/HeapSwitch code with the E32 heap min/max and 4096-byte growth.
It checks the returned allocator and gates application creation on success.
On the host with the supplied RH-29 data, native RHeap is created at 0x20000000;
Shadowkey's factory returns 0x20000080 with vtable 0x120fd9bc. This is execution
of the supplied program/ROM, not a fabricated application object. The owner's
0.3.15 New 3DS logs confirm the same heap and application-object results, all
CPU/E32/kernel fixture flags passing and zero recorded CPU faults. The complete
diagnostic takes 3613 ms; this is not a gameplay speed measurement. Evidence:
validation/0.3.15/device-v0315-*.log and hardware-v0315-result.json. The additional
512-object allocation stress test remains host-only.

The probe adds kernel checks to symbian-probe.log. Y uses the same private data
and records THREAD_HEAP and APPLICATION_OBJECT in symbian-loader.log. No GUI,
phone OS boot, full scheduling, filesystem/window/audio services, recursive game
DLL initialization, EKA2 or SIS support is claimed. GAMECOMMS/SIMKIN are still
unresolved and no playable Shadowkey scene is reached.

## Full ROM/link path confirmed on New 3DS — 2026-11-05

The owner supplied complete 0.3.14 logs. All CPU/E32 fixture flags pass, and
the real Shadowkey test validates its image, maps the full RH-29 ROM and links
324/415 imports. Guest mappings total 19,347,140 bytes. The DLL entry returns;
the application factory reaches EUSER and stops at EKA1 SVC 0x6c (Heap), with
no recorded guest memory fault. The whole diagnostic completes in 3542 ms;
that is not a game FPS benchmark or the isolated ROM-read duration.
Evidence: validation/0.3.14/device-v0314-*.log and hardware-v0314-result.json.
GAMECOMMS and SIMKIN remain unresolved; they are not supplied ROM descriptors.
This confirms the loader/ROM/CPU path, not a phone OS or playable game.

Next kernel work must initialize the EKA1 thread allocator before application
creation. Pinned upstream kernel/libmanager.cpp::build_eka1_thread_bootstrap_code
calls EUSER ordinal 166 (ChunkHeap) with min/max/grow parameters, then ordinal
1127 (HeapSwitch) before DLL thread-attach entry points. kernel/svc.cpp::heap
returns the current thread-local allocator pointer. This requires the matching
thread/chunk/handle environment and EKA1 SVC return convention, plus subsequent
game-DLL loading and services. No implementation of those is claimed here.

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
