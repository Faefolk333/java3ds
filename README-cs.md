# Java3DS 0.3.6 — privátní alpha

Nově v 0.3.6: Nokia drawPixels bez rotace používá přímé nativní kopírování
místo vytváření dočasných obrázků. Micro3D posílá celé sady trojúhelníků do nativního rendereru a
maže jen používanou část hloubkového bufferu. Stejný 35sekundový test Blades na
PC vykreslil 726 snímků místo 602 a spotřeboval asi o 20 % méně času CPU.
Výkon na 3DS je ještě nutné změřit.

L+R nebo dotykové MENU > Stereo 3D: Off / Models / Layers / Auto. Přibyla také
síla 1–6; efekt ovládá posuvník 3D konzole. Výchozí stav je Off. Models používá
hloubku Micro3D/M3G, Layers skutečné vrstvy LayerManager, Auto obojí. Efekt je
experimentální reprojekce hotového obrazu: odkryté části ploch jen přibližuje.
Hloubka se přenáší i s pomocnými obrázky, jejich kopiemi a přes dvoumístnou
cache getRGB/drawRGB. Libovolné vlastní kopírování pixelových polí ji stále může
ztratit. Ze slitého obrázku nelze poznat původní vrstvy. Pro nejvyšší výkon ponech 3D vypnuté. Volby se ukládají pro hru.

Banner obsahuje viditelné BUILD 0.3.6. Po instalaci konzoli úplně restartuj.
Pokud označení chybí, může HOME Menu ještě ukazovat starý banner; pokud je vidět,
ale pohyb je chybný, je problém v převodu nebo rámování animace. Původní modelová
animace a dodaný WAV zůstávají. Originální Blender soubor na ploše se nemění.

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
