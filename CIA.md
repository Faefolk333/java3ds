# Java3DS CIA and animated HOME Menu banner

Install `java3ds.cia` using FBI on a homebrew-enabled 3DS. The CIA embeds
RomFS, the MIDI SoundFont, adapters and profile resources. No separate runtime
copy is needed. Games/settings/saves and the optional private GitHub token
remain under `/3ds/java-3ds` on SD, shared with the 3DSX edition.

## QR installation

The repository stays private. A standard FBI QR cannot use the token belonging
to Java3DS, so the QR installer serves only the CIA from your PC on the LAN.
On this PC, run `Start-QR-Install.ps1` in the delivered release folder. Keep
that PC awake and connect the 3DS to the same home network. In FBI, choose
**Remote Install > Scan QR Code**, scan `Java3DS-install-QR.png`, and confirm.
The server exits after 24 hours. Starting it again produces a new QR.
Only the CIA is served; there is no directory listing or GitHub credential.
This initial installation is separate from private GitHub updates inside the app.

Generic invocation (Python with qrcode and Pillow):

```text
python tools/serve_cia.py --file dist/java3ds.cia --host YOUR_LAN_IP --output dist/qr
```

## Build

Build the Java adapter/runtime as documented in README, then run
`tools/setup-cia-tools.ps1` to download pinned build tools. Set `PYTHON` to your
Python executable if `python3` is not on PATH. With devkitARM,
libctru and the same portlibs as the 3DSX build, run `bash build-cia.sh`.
The checked-in generated banner is sufficient; Blender is not needed for a
normal application rebuild. Output: `work/arm/java3ds.cia`, title ID
`000400000F3A3D00`, product code `CTR-P-J3DS`, title version 56 (0.3.8).
Keep this title ID for future updates. The executable supports Old/New 3DS;
64 MiB/124 MiB memory profiles are encoded in its exheader. The CIA build
reserves 4 MiB of linear memory and caps the regular heap at 48 MiB (Old) / 80 MiB (New), with resource-limit checks.

## Rebuild the custom banner

The supplied Desktop Blender scene has a fixed camera: sampled frames 1/30/60/90/120
have identical camera matrices. Its 120 frames at 24 fps form a five-second loop.
Model transforms are baked into the fixed HOME Menu perspective and closed by a
matching final pose. Its source tracks use 300 ticks at 60 Hz; the CGFX
container cycle is 2400 ticks to align source motion and orbit compensation.

`assets/banner/source.blend` is now a compact derived scene imported from the
optimized GLB; the user's Desktop original was not modified. `banner.glb`,
`geometry.json` and `manifest.json` reproduce the shipped asset. Its closed meshes use single-sided triangles. The 0.3.7 billboard-only parent
failed on console and is no longer used. A separate root rotation compensates
an approximately eight-second observed orbit; original five-second source
tracks use repeat curves. The combined cycle is 40 seconds. This timing still
needs hardware verification and may drift with microphone/motion inputs.
There is no version badge. The CGFX is 264,216 bytes, below 512 KiB.
Serialized dictionary and TRS/source comparisons are build gates. Glass shells use alpha
blending, bright specular material and a Phong lookup table. HOME Menu cannot
reproduce Blender's physical refraction. Preview PNGs are an approximation.

For a new original scene, run Blender 5.2 with `--background --factory-startup
--disable-autoexec ORIGINAL.blend --python tools/export_banner.py`. Copy the
resulting work/banner-v034/banner.glb and geometry.json into assets/banner,
then run tools/save_banner_source.py in a fresh Blender background process.
Use https://github.com/skyfloogle/pycgfx at commit
`1f78850086f3a77c41e07162e842f97a5bf3c18a` in `work/cia-tools/pycgfx`.
The conversion environment needs `gltflib==1.0.13` and Pillow. Then:

```text
python tools/convert_banner.py
bannertool makebanner -ci assets/banner/banner.cgfx -a assets/banner/banner.wav -o assets/banner/banner.bnr
```

The supplied 48×48 icon is used by both CIA and 3DSX installers. The HOME Menu
banner includes the supplied 1.846-second stereo WAV, normalized to -1 dBFS and resampled
to PCM16/32000. `banner-source.wav` preserves the supplied original;
`prepare_banner_audio.py` reproduces processing.
Navigation sounds play independently inside Java3DS itself.

`Facing anchor` stays an identity parent in the source GLB. The converter adds
its separate compensation channel in CGFX. No billboard is applied to animated
mesh bones, so their authored rotations are not flattened. `banner_motion.py`
compares serialized transforms with the source at 640 samples assuming the
specified external orbit, and explicitly does not verify hardware timing.

## Recovery from the 0.3.4 HOME Menu crash

Open FBI directly without highlighting Java3DS, and install the replacement
CIA over the existing title. Do not delete game/save folders. The replacement
uses the same title ID and version 56. Dumps 29/30 both identify process `menu`,
PC 0x187d58, LR 0x20b940, FAR 0x8; these are HOME Menu addresses, not VM code.
The malformed dictionary is independently reproduced in the shipped 0.3.4 CGFX;
the precise HOME function is not symbolicated. See validation/0.3.5.

## Verification

CTRTool checks content, ExeFS and RomFS hashes; extracted resources are compared
to source. Ticket/TMD/access-descriptor signature failures are expected for
an unsigned homebrew CIA and require the same custom firmware used for FBI.
Physical HOME Menu playback, first launch and CIA self-updating still require
a console test. See VALIDATION.md for the exact completed checks.

## Implementation references for 0.3.8

- Rigid-submesh billboard evaluation uses the bound bone:
  https://github.com/gdkchan/SPICA/blob/master/SPICA.Rendering/Mesh.cs
- libctru documents core 2 permission and Old/New thread restrictions:
  https://github.com/devkitPro/libctru/blob/master/libctru/include/3ds/thread.h

The eight-second compensation period comes from this console user's estimate,
not from a verified universal HOME Menu timing constant.
