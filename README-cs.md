# Java3DS 0.3.3 — privátní alpha

Nainstaluj `java3ds.cia` přes FBI; nahradí předchozí Java3DS se stejným ID.
Hry, nastavení a uložené pozice zůstávají v `/3ds/java-3ds` na SD.
Pro Homebrew Launcher rozbal instalační ZIP do kořene SD.

QR: FBI > Remote Install > Scan QR Code. Počítač musí zůstat zapnutý a 3DS
musí být ve stejné domácí síti. QR platí 24 hodin. `Start-QR-Install.ps1`
v této složce spustí nový server a vytvoří nový QR.

Nové v 0.3.3:

- Ošetřené selhání alokace z dumpu Age of Heroes III, méně dočasných obrázků
  a méně práce při každém vykreslení. Úplné odstranění pádu ověř až na konzoli.
- Worms: opravené dlouhé obrázkové pásy se sprity; na PC jsou vidět i těla červů.
- Hudba a efekty MIDI/PCM WAV se přehrávají nezávisle. AMR/AAC/MP3 zatím ne.
- Experimentální skutečné 3D: Micro3D v3 (Blades and Magic) a M3G
  (Outland, Gladiator). Výkon na 3DS ještě není změřen; některé efekty chybí.
- Discworld se v testu dostal do úvodního dialogu. Doplněny RMS enumerace
  a základní formuláře. Krátké testy nejsou zárukou dohratelnosti.
- Rozlišení LocoRoco se rozpoznává jako 240×320 z LGE manifestu.
- Native centruje obraz; Fill screen vyplní 400×240 roztažením hotového obrazu.
  Poměr stran se může změnit. Okraje uvnitř grafiky hry se neodstraňují.
- Spodní klávesy reagují vizuálně pouze na dotyk. Plynulejší aktualizace paralaxu.
- Senzory jsou přístupné hrám přes JSR-256 a Sony API. Gyroskop a akcelerometr
  jsou odlišné senzory; hra je musí sama používat. Tento LocoRoco build je nevolá.

Širší kamera je stále profil konkrétní hry (PoP Classic); univerzální Fill screen
nemění herní svět. Ovládání a skrytí spodního displeje nastavíš v Controls.
Banner se při této aktualizaci neměnil.

Soukromé Wi-Fi aktualizace používají token na SD v
`/3ds/java-3ds/github-token.txt`; postup je v UPDATES.md. Lokální QR funguje
bez tokenu. Zveřejněné GitHub vydání a místní testovací sestavení se mohou lišit.

Testováno na počítači, fyzická konzole zatím ne. Po chybě zkopíruj runtime.log,
output.log a nový Luma dump před dalším spuštěním. Podrobnosti: VALIDATION.md.
