# Wi-Fi updates and temporary public testing

Channel: https://github.com/Faefolk333/java3ds . The owner's current policy is
to make the repository public for a test window, keep it public until the owner
says testing has finished, then return it to private. This is not authorization
for a permanent public release. There is no automatic timeout.

## Updating inside Java3DS 0.3.10+

Connect the console to Wi-Fi. Java3DS checks at startup; **Y** in the library
opens updates, and **A** checks/installs an offered newer build. Exit and reopen
after installation. Anonymous checks/downloads work while the repository is
public. No token file is required for the test window. While it is private the
app reports that the test channel is private; it cannot bypass GitHub access.

**Bootstrap from 0.3.9:** that installed version stops before making a network
request if no token file exists. Install 0.3.10 once using CIA/QR or replace the
3DSX manually. Subsequent public-window updates use the in-app updater.

A legacy optional `/3ds/java-3ds/github-token.txt` read-only token still works
for private access. Requests try anonymously first; authentication is retried
only for API access failures and only at `api.github.com`. No shared credential
is embedded in the application, QR, source or release.

## Preparing a test build

1. Increase app/UI version and CIA title version; compile, test and package.
2. Commit and tag matching sources. Upload verified CIA, 3DSX, installation ZIP,
   complete corresponding sources/licenses and SHA256SUMS while still private.
3. Publish the test release with GitHub's Pre-release flag **unchecked**, because
   the updater uses `/releases/latest`. Name it clearly as a test build.
4. Confirm asset sizes and GitHub `sha256:` digests, then open the authorized
   public test window. Verify the anonymous latest-release and asset paths.
5. Keep it public until the owner says testing is finished; then restore private
   visibility. A later permanent release needs their instruction.

Asset names must be **java3ds.cia** and **java3ds.3dsx**. CIA uses title ID
`000400000F3A3D00`; its own CIA is verified before system installation. HOME is
temporarily disabled during replacement; games remain unavailable until restart.
Games/settings/RMS saves remain on SD. The CIA path needs console testing.

3DSX writes an `.update` download, verifies size/SHA-256/magic, backs up the
installed executable as `.previous`, then replaces it. Verification failure
retains the old application. If power fails between renames, restore `.previous`.
The token is sent only to the API, never to redirected asset hosts.

HTTPS uses libcurl/mbedTLS with certificate and hostname verification. Only
GitHub/API/asset CDN destinations are allowed. Downloads and metadata are bounded.
Shutdown cancels the network worker and waits at most two seconds; a worker that
cannot unwind causes process recovery instead of freeing its live state.

Bundled CA roots (collected through verified system trust; reproducible with
`tools/collect_update_roots.py`):

* USERTrust ECC: `4ff460d54b9c86dabfbcfc5712e0400d2bed3fbc4d4fbdaa86e06adcd2a9ad7a`
* ISRG Root X1: `96bcec06264976f37460779acf28c5a7cfe8a3c0aae11a8ffcee05c0bddf08c6`

The production updater can be built with test-only host service/thread adapters
using `tests/build-updater-network-check.sh` (MinGW libcurl required). It verifies
real anonymous HTTPS, metadata/SHA/size, 3DSX installation, backup and save
preservation; this does not emulate the console CIA installer or Wi-Fi stack.
