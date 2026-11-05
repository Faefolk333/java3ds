# Symbian investigation — no executable backend yet

Java3DS executes Java ME bytecode. It does not execute Symbian SIS/SISX/APP
programs. The inventory in `validation/0.3.10/symbian-audit.json` was produced
without installing or running a package.

* Shadowkey and Tomb Raider include EKA1 / ARMv4 E32 applications for N-Gage.
* Asphalt Urban GT 2, Call of Duty, Splinter Cell: Chaos Theory, Tony Hawk's
  Pro Skater and Worms World Party archives contain native `system/apps/*.app`
  files, rather than JARs. Several supplied archives are demos or review builds.
* Hero of Sparta and Prince of Persia HD contain native S60v3-or-later SISX
  executables. Their controller metadata names EXE payloads, not embedded JARs.
* No device ROM, RPKG or VPL was supplied, including in the added archives.

EKA2L1 needs a compatible device/OS image in addition to the game:
https://eka2l1.github.io/quickstart/basic/installdevice/
Firmware is the phone's operating system; a game APP/SIS does not replace it.
No firmware is bundled or downloaded by Java3DS.

Upstream https://github.com/EKA2L1/EKA2L1 has no Nintendo 3DS target. A port
needs platform/kernel integration, a CPU execution strategy, graphics adaptation
and memory-budget work, followed by device-image and game tests. The DualStick
fork adapts Android input; it supplies no 3DS execution backend. Upstream was
inspected locally; no EKA2L1 engine is linked or distributed in this build.

`tools/symbian_audit.py` reads E32 headers and bounded SIS controller metadata;
optional `py7zr` lists 7z members. Originals remain unchanged. The report makes
no playability claims.
