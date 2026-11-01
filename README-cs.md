# Java3DS 0.3.5 — privátní alpha

CIA nainstaluj přes FBI; zachová hry a uložené pozice v /3ds/java-3ds.
Pro Homebrew Launcher rozbal instalační ZIP do kořene SD karty.

Oprava pádu při označení ikony v HOME Menu: banner 0.3.4 obsahoval po řazení
průhledných modelů poškozenou tabulku názvů kostry. Nový export ji přestaví a
kontroluje všech 83 vyhledání názvů v hotovém CGFX. Předchozí banner tímto testem
neprojde. Animace i průsvitné materiály zůstávají; nově je přibalený dodaný zvuk
java 3ds banner.wav (1,85 s). Opravu je ještě nutné ověřit na skutečné konzoli.

Otevři přímo FBI a nainstaluj novou CIA přes Remote Install > Scan QR Code,
případně ze SD. Před instalací nenajížděj na rozbitou ikonu Java3DS. Nová verze
přepíše stejný titul, hry a uložené pozice na SD zůstanou zachované.

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
polovinu RAM. U Blades, které MIDI nepoužívá, se velká banka nenačte vůbec.
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
