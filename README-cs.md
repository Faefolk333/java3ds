# Java3DS 0.3.7 — privátní alpha

GameCanvas nyní zveřejní hotový obraz bez druhého volání kreslení hry. V PC testu
Moorhuhn funguje menu, kuřata, zaměřovač a střelba. Skryté plátno nemůže přepsat
obraz aktuální hry. Yeti Sports dostalo správné kreslení úzkého výseku golfové hole.

Micro3D nově respektuje poloprůhledné, sčítací a odečítací efekty a posun grafiky.
Černé pozadí sčítacích efektů nepřekryje scénu. Přibylo časné odmítnutí pixelů mimo
trojúhelník. Výkon na fyzickém 3DS je potřeba znovu ověřit.

Zvuk podporuje MIDI, PCM WAV a nově AMR-NB, chybějící PlayerListener a mute API.
Opravilo se také zkreslení záporných PCM vzorků. MP3, AAC a AMR-WB zatím nejsou
podporované. Dodaná Soul of Darkness sama vypíná mnoho efektů číslem -1 a
nekonečnou smyčku mění na jediné přehrání. Toto chování hry se nepřepisuje.
PC test Blades 320 × 240 se zapnutou hudbou ji spustil a vytvořil zvuk.

3D je výchozí Auto, aktivní podle posuvníku konzole. Layers kromě LayerManager
zkouší odhad hloubky běžných obrázků a spritů, Auto přidává hloubku modelů.
Witcher a Soul of Darkness v testech již předávají různé hloubky. Jde o odhad,
ne o obnovenou geometrii. Orcs & Elves II předává hotový obraz z vlastního rendereru;
pro skutečnou hloubku jeho scény je stále potřeba zvláštní adaptér. Nově uložené
Off zůstane zachované, starý výchozí stav Off se jednou převede na Auto.

V herním menu je CRT: Off / Soft scanlines / Soft grid. Jemné filtry ovlivňují jen
hru a ukládají se pro titul. L+R nebo dotykové MENU otevře nastavení.

Banner už neobsahuje číslo verze. Přidaný pevný rodič potlačuje otáčení celého
modelu v HOME Menu a zachovává původní čtyři animační kanály. Zvuk je zesílený
na špičku -1 dBFS a převedený na 32 kHz. Originály na ploše se nemění. Přesný pohyb,
načasování zvuku a vzhled materiálů musí potvrdit konzole.

CIA nainstaluj přes FBI. Hry a pozice v /3ds/java-3ds se zachovají. Pro Homebrew
Launcher rozbal instalační ZIP do kořene SD karty.

Ovládání spodního displeje z předchozí aktualizace:

- LEFT a RIGHT jsou dotykové softkeys. Prostřední MENU nebo L+R otevře herní menu.
- Menu pozastaví Java vlákna i herní audio. Nabízí pokračování, Native/Fill screen,
  časovač displeje, ukončení hry do knihovny a zavření Java3DS.
- START+SELECT stále ukončuje hru přímo. Změna displeje během hry mění škálování
  hotového obrazu; případná změna PoP kamery se uplatní při příštím spuštění.
- Zhasnutí spodního podsvícení: Never / 15 / 30 / 60 / 120 sekund, výchozí 30.
  První dotyk displej jen probudí. Start bottom screen: Asleep začne se zhasnutým
  displejem a také dovoluje probuzení dotykem. Nastavení se ukládá pro každou hru.

Nativní výpočty urychlují běžné M3G texturování a Micro3D transformace modelů.
Jde stále o softwarové 3D. Dump 28 odhalil další selhání alokace při načítání
tříd; nově je ošetřené. MIDI banka se načte až při spuštění MIDI a vzorky zabírají
polovinu RAM. U hry bez spuštěného MIDI se velká banka nenačte.
Odstranění systémového pádu a výkon musí potvrdit nové hraní na konzoli. Na PC jsou ověřené scény Blades, Outland a Orcs & Elves II.
Worms se v PC testu načetly a vykreslily celé červy; hlášený problém na konzoli
bez aktuálního logu zatím není potvrzený jako odstraněný.

Nová ikona a banner vycházejí z dodaných souborů. Kamera byla v originálu pevná;
loop trvá 5 sekund (120 snímků při 24 fps). Průsvitnost a lesk CGFX přibližují sklo,
bez skutečné Blender refrakce. Původní soubor na ploše zůstal nedotčený.
Navigace, potvrzení a dokončení aktualizace používají dodané WAV zvuky.

Spore Creatures ve složce obsahuje pouze malý JAD, nikoli herní JAR. Bez JARu se
nespustí. Nabídka nyní odlišuje JAR/JAD a nepíše obecnou hlášku o chybějícím JARu
u každé hry. Předchozí logy se zachovávají jako runtime.previous.log a output.previous.log.

QR: FBI > Remote Install > Scan QR Code. Počítač musí zůstat zapnutý a 3DS být
ve stejné domácí síti. Lokální QR platí 24 hodin. Start-QR-Install.ps1 vytvoří nový.
Wi-Fi aktualizace v aplikaci používají publikovaná GitHub vydání a token na SD;
postup je v UPDATES.md. Místní testovací sestavení může být novější než vydání.

Podrobnosti o testech a omezeních: VALIDATION.md a COMPATIBILITY.md.
