# Compatibility work — 0.3.2

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

## 3D backend inventory

Both supplied Blades and Magic JARs use Mascot Capsule Micro3D v3: MBAC/TRA
figure and animation decoding, textures, perspective transforms and rendering.
Gladiator 3D and Outland 3D use JSR-184/M3G: a distinct scene graph, loader,
animation system and renderer. These require real backend ports; API stubs
would only move failure later or leave invisible scenes. Their launch guard
remains active. The next 3D work should target one backend and one game first.
Symbian needs another runtime/emulator and is outside this MIDP patch.

See VALIDATION.md for tests actually performed. No physical-console success
is claimed. The animated banner is deferred for a separate framing/motion review.
