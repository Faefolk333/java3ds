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
`tools/setup-cia-tools.ps1` to download pinned build tools. With devkitARM,
libctru and the same portlibs as the 3DSX build, run `bash build-cia.sh`.
The checked-in generated banner is sufficient; Blender is not needed for a
normal application rebuild. Output: `work/arm/java3ds.cia`, title ID
`000400000F3A3D00`, product code `CTR-P-J3DS`, title version 51 (0.3.3).
Keep this title ID for future updates. The executable supports Old/New 3DS;
64 MiB/124 MiB memory profiles are encoded in its exheader. The CIA build
reserves 4 MiB of linear memory and caps the regular heap at 48 MiB (Old) / 80 MiB (New), with resource-limit checks.

## Rebuild the custom banner

`assets/banner/source.blend` is the user's unchanged original. It contains
120 frames at **24 fps**, not 120 fps. The derived animation is a five-second
loop. HOME Menu receives a native CGFX skeleton animation, 300 ticks at 60 Hz;
that does not guarantee 60 rendered frames per second on the console.

1. Use Blender 5.2+ with the glTF exporter. Run it in background/factory mode
   with `--disable-autoexec assets/banner/source.blend --python tools/export_banner.py`.
2. In `work/cia-tools/pycgfx`, check out commit
   `1f78850086f3a77c41e07162e842f97a5bf3c18a` from
   https://github.com/skyfloogle/pycgfx . Install `gltflib==1.0.13` and Pillow.
3. Run `python tools/convert_banner.py` and then bannertool:
   `makebanner -ci assets/banner/banner.cgfx -a assets/banner/silence.wav -o assets/banner/banner.bnr`.
4. Rebuild the CIA. `assets/banner/manifest.json` records geometry and animation.

The source's dense geometry is simplified to 2,718 vertices. The CGFX is
441,336 bytes, below HOME Menu's 512 KiB limit. Both animated objects retain
their motion and loop; camera normalization matches the fixed banner camera.
The procedural Electromagnetic Shield shader is approximated by mint chrome
with emission because HOME Menu cannot run Blender shader nodes. No banner
sound was supplied, so its audio is silent. Blender preview renders are an
approximation, not a screenshot of the actual HOME Menu renderer.

## Verification

CTRTool checks content, ExeFS and RomFS hashes; extracted resources are compared
to source. Ticket/TMD/access-descriptor signature failures are expected for
an unsigned homebrew CIA and require the same custom firmware used for FBI.
Physical HOME Menu playback, first launch and CIA self-updating still require
a console test. See VALIDATION.md for the exact completed checks.
