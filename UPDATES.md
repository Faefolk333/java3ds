# Private Wi-Fi updates

Repository: https://github.com/Faefolk333/java3ds (private).

## One-time setup on the SD card

Create a **fine-grained personal access token** in GitHub:
https://github.com/settings/personal-access-tokens/new

* Resource owner: `Faefolk333`.
* Repository access: **Only select repositories → java3ds**.
* Repository permissions: **Contents → Read-only**. Metadata read access is implicit.
* Choose an expiry date. When the token expires, replace the SD file.

Save only the token, on one line, in the plain UTF-8/ASCII file:
`/3ds/java-3ds/github-token.txt` on the SD card. No quotes or UTF-8 BOM.
Do not put the token in a commit, ZIP, release notes, or chat.
The installed application, sources and installer do not contain a token.

Start Java3DS with Wi-Fi connected. When that file exists, the library checks
for a new release at startup. **Y → Wi-Fi updates** also checks manually.
**A** installs an offered newer version; **B** postpones it.
Exit and reopen Java3DS after installation. The current running process does
not replace itself in memory.

The first v0.3.0 installation will report up to date. The automatic path can
install a later release such as v0.3.1 or v0.4.0.

## Publishing a later version

1. Increase the version in `native/updater.cpp`, the UI and packaging metadata.
2. Build and validate the application. Commit sources and tag `vX.Y.Z`.
3. In this private repository, publish a release for that tag. Leave the
   GitHub **Pre-release** flag unchecked: this repository is the private update
   channel, and the updater uses GitHub's latest-release endpoint.
4. Attach the standalone 3DSX with the exact asset name **java3ds.3dsx**.
   It must include the complete RomFS, runtime, SoundFont, profiles and CA bundle.
   Also attach the SD installation ZIP and optionally the full source ZIP.
5. GitHub's release asset metadata must contain a `sha256:` digest. Releases
   without a digest, invalid tag, or compatible asset are rejected.

The updater uses HTTPS with certificate and hostname verification, bounded
downloads, SHA-256, size and 3DSX-magic checks. The GitHub bearer token is sent
only to `api.github.com`, never to the asset CDN after a redirect. GitHub and
GitHub asset CDN hosts are the only accepted destinations.

It writes `<installed executable>.update`, verifies it, renames the installed
executable to `.previous`, then installs the download. Existing games, settings,
RMS saves and the token are preserved. It installs over the executable path
supplied by Homebrew Launcher, including the older `java_pocket.3dsx` filename.
If a power loss happens between the two renames, restore the `.previous` file
on the SD card. A failed verification leaves the installed application intact.

The private repository needs to remain accessible to the SD token. A future
public release channel would require a separate public repository or a change
of access architecture; the application never carries a shared private token.

## TLS roots

The bundled roots came from locally verified TLS chains for the API and asset
hosts, using the system trust store; no verification was disabled.

* USERTrust ECC Certification Authority:
  `4ff460d54b9c86dabfbcfc5712e0400d2bed3fbc4d4fbdaa86e06adcd2a9ad7a`
* ISRG Root X1:
  `96bcec06264976f37460779acf28c5a7cfe8a3c0aae11a8ffcee05c0bddf08c6`

`tools/collect_update_roots.py` reproduces the collection with Python 3.13+.
Review the resulting CA names and fingerprints before publishing an update.
Transport uses devkitPro's libcurl + mbedTLS, avoiding the console's old HTTPS
service. Real console Wi-Fi/TLS and SD installation still need hardware testing.

## CIA channel (0.3.1+)

The installed HOME Menu build selects `java3ds.cia` from the same private
release. It verifies SHA-256 and size, then checks Title ID
`000400000F3A3D00` with the system installer before installation. RomFS is
closed before replacement. After success, exit and reopen the application;
returning to games is disabled until restart. The CIA replaces the installed
title on SD; the `.previous` backup applies only to the 3DSX build.
No GitHub token is embedded in either application or in the QR code.
The CIA self-update path still needs a physical-console test.
