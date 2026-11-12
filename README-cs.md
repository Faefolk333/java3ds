# Java3DS 0.5.0 — větší testovací balík Symbianu

Nově: skutečné zachycení Leave a nativní úklid přes původní ROM, vnořené TRAP
rámce, lokální data DLL, kontext plánovače, pojmenované synchronizační objekty
se sdílenými handly a ukončení emulovaného vlákna. Na PC prošly běžné i vnořené
chyby, pořadí úklidu, obnova registrů a 512 cyklů přidělení/uvolnění paměti.
Původní plánovač obsloužil připravené testovací události podle priority, předal
Leave do RunError a odregistroval objekty. Nejde zatím o plné asynchronní služby.

Přibyl kontrolovaný přístup k cestám v ROMu a UTF-16 informace procesu. Původní
AppRun se na PC dostane k požadavku na FileServer. Zde zatím skončí: souborový
server a IPC, grafické/okenní/fontové a zvukové služby, plný běh procesů,
EKA2/SIS a herní obrazovka ještě nejsou hotové. Java hry se v tomto balíku nemění.
Vaše logy 0.5.0 nyní potvrzují tyto kroky na New 3DS: všechny kontroly prošly,
16 cyklů TRAP i plánovač fungují a CPU hlásí nula chyb. Diagnostika skončila za
6460 ms u očekávaného FileServer. Zátěžový test 512 cyklů zůstává výsledkem z PC.

Aktualizujte přes Wi-Fi, znovu otevřete Java3DS a v Homebrew Launcheru spusťte
Symbian Probe 0.5.0 -> Y. Použijte stejná soukromá data z 0.4.0; nový ZIP není
potřeba. Pošlete symbian-probe.log a symbian-loader.log z /3ds/java-3ds/.
Očekáváme všechny příznaky NATIVE_TRAP a NATIVE_CONTEXT = 1. Záznam
ROM_APPRUN_SERVICE name=FileServer označuje další dosud chybějící službu;
není to potvrzení spuštění hry. Soukromé ROM/herní soubory nejsou na GitHubu.

## Previous history

# Java3DS 0.4.1 — testovací sestavení

Vaše logy z New 3DS potvrzují celý test 0.4.0: všechny importy, obě herní DLL,
paměť i správné UID aplikace. Nově test také zjistí pořadí inicializace knihoven
přímo z ROMu a spustí jejich vstupní funkce. Na PC prošlo 114 ze 114 knihoven.
Potom původní EUSER vytvoří CTrapCleanup, tedy nativní úklidový zásobník a
obsluhu chyb vlákna. Vaše logy 0.4.1 nyní potvrzují stejný výsledek na New 3DS:
všech 114 knihoven i cleanup prošly, všechny kontroly mají výsledek 1 a CPU
hlásí nula chyb. Celá diagnostika trvala 5589 ms; nejde o měření rychlosti hry.

Aktualizujte přes Wi-Fi, znovu otevřete Java3DS a v Homebrew Launcheru spusťte
Symbian Probe 0.4.1 -> Y. Platí stejná soukromá data z 0.4.0; nový ZIP není třeba.
Logy stále najdete v /3ds/java-3ds/symbian-probe.log a symbian-loader.log.
Očekávané záznamy: trap=1, ROM_INITIALIZATION ready=1 completed=114/114 a
NATIVE_CLEANUP ready=1. Po vytvoření cleanup objektu se adresa aplikace změnila;
UID má stále souhlasit. Zatím nevzniká herní obrazovka: aplikační prostředí,
plánování, služby souborů/grafiky/zvuku, zpracování Leave a SIS ještě nejsou hotové.
Chování Java her se nemění; soukromé ROM/herní soubory nejsou ve veřejném vydání.

## Previous history

# Java3DS 0.4.0 — testovací sestavení

Samostatný Symbian test nyní načítá také herní EKA1 DLL včetně jejich závislostí.
Na PC připojí všech 415 importů Shadowkey a dalších 284 importů knihoven
GAMECOMMS/SIMKIN. Obě knihovny dokončí inicializaci vlákna a vytvořený objekt
aplikace vrátí správné UID. Váš test na New 3DS nyní potvrzuje stejný výsledek:
všechny kontroly prošly a celá diagnostika trvala 4610 ms.

Po aktualizaci znovu otevřete Java3DS. Nový soukromý ZIP Shadowkey rozbalte do
kořene SD; přidává game-libs/GAMECOMMS.DLL, SIMKIN.DLL a osm hlaviček ROM DLL.
Potom v Homebrew Launcheru spusťte Symbian Probe 0.4.0 -> Y. Logy jsou nadále
/3ds/java-3ds/symbian-probe.log a symbian-loader.log. Soukromý ZIP patří pouze
na vaši SD, není na GitHubu ani ve veřejných instalačkách.

Zatím jde o diagnostiku: nevzniká herní obrazovka. Zbývá dokončení inicializace
ROM knihoven, aplikační prostředí, plánování a služby souborů, grafiky a zvuku;
EKA2 a SIS instalátor rovněž nejsou hotové. Předchozí krok 0.3.15 s haldou a
objektem aplikace už prošel na vašem New 3DS. Chování Java her se nemění.

## Previous history

# Java3DS 0.3.15 — testovací sestavení

První část jádra EPOC6 nyní obsluhuje paměťové bloky, vybrané služby vláken,
semafory, mutexy a výběr haldy. Samotný RHeap vytváří původní kód EUSER z vašeho
ROMu. Na PC i na vašem New 3DS už Shadowkey vytvoří objekt aplikace a překročí
zastavení na Heap. Všechny kontroly CPU/E32/kernelu na konzoli prošly; celá
diagnostika trvala 3613 ms. To není měření rychlosti hry. Použijte stejná soukromá
data: po aktualizaci otevřete Java3DS a spusťte Symbian Probe 0.3.15 -> Y.
Logy přidávají kernelové kontroly, THREAD_HEAP a APPLICATION_OBJECT.
Zatím nevzniká herní obrazovka; služby systému, plánovač, grafika/zvuk a SIS
instalátor ještě zbývají. Změna je v samostatném testu, Java hry jsou beze změny.

Předchozí změny 0.3.14:


Ve vašem testu 0.3.13 prošel start, CPU i všechny kontroly E32 a paměti.
Čtení ROMu bylo přerušeno tlačítkem START. Verze 0.3.14 ukazuje procenta,
načtené KiB a uplynulý čas. Čte ROM po 64 KiB a po zrušení už nepokračuje
spouštěním programu. B zruší jen diagnostiku, Y ji zopakuje a START ukončí
aplikaci. Log uvádí fázi, množství načtených dat, čas a zdroj zrušení.
Použijte stejná soukromá data Shadowkey. Plné načtení ROMu na konzoli ještě
potřebujeme ověřit; hratelný Symbian ani nové služby jádra zatím nepřibyly.

Předchozí změny 0.3.13:


Opraven pád samostatného Symbian Probe před startem. Dump 31 odpovídá přesně
kódu 0.3.12: systémový start žádal přibližně 121,59 MiB běžné haldy, jejíž
adresní prostor má 96 MiB. Omezení použité už u CIA je nyní připojeno také
k hlavní 3DSX a testu. Nová verze potřebuje ověřit start na konzoli.
Po aktualizaci znovu otevřete Java3DS, pak v Homebrew Launcheru spusťte probe
s označením 0.3.13. Data soukromého ZIPu 0.3.12 platí dál. Tlačítko Y spustí
stejnou diagnostiku a uloží symbian-loader.log. Hratelný Symbian zatím není.

Předchozí změny 0.3.12:


Verze 0.3.12 rozšiřuje samostatný Symbian Probe o načítání nativních EKA1
programů a ROM pro N-Gage. Kontroluje adresy, importy, relokace a oddělení paměti
kódu, dat a zásobníku. Na PC načte Shadowkey a připojí 324 ze 415 importů k
opravdovému kódu dodaného ROMu. Běh se zastaví na dosud chybějící službě Heap
jádra Symbianu. **Hru ani telefonní systém zatím na 3DS nespouští.**

Po aktualizaci otevřete jednou hlavní Java3DS; opravena je chyba, kvůli které
0.3.11 vytvořila jen prázdnou složku testu. Potom spusťte Java3DS Symbian CPU
Probe v Homebrew Launcheru. Vlastní testy zapisuje do symbian-probe.log.
Tlačítko Y navíc ověří data v /3ds/java-3ds/symbian (test.app, SYM.ROM,
rom-libs/*.DLL) a zapíše /3ds/java-3ds/symbian-loader.log. Soukromý testovací
ZIP z vašich souborů patří pouze na vaši SD; není součástí veřejného vydání.
Nové načítání potřebuje ověřit na konzoli. Samotný CPU test 0.3.11 už na vašem
New 3DS prošel bez chyby.

Předchozí opravy verze 0.3.11:


Opraven čas VM na konzoli: osGetTime počítá od roku 1900, systémové časové
limity vláken a Java od roku 1970. Dříve se tak krátká čekání mohla změnit v
prakticky nekonečná nebo přetéct. Testovací VM s napodobenými hodinami 3DS nyní
prochází 22 testy dvakrát včetně porovnání s nezávislým časem PC a timed wait.
Gladiator v host testu došel do boje, LOTR a Orcs II do hry, Resident Evil za
výběr mise do výběru výbavy. Výsledek na fyzické konzoli je stále nutné ověřit.
Oba dodané Orcs II JARy jsou obsahově stejné. Uložené pozice se zachovávají.

V instalačním ZIPu je navíc samostatný Java3DS Symbian CPU Probe pro Homebrew
Launcher. Portuje CPU interpret EKA2L1 na ARM11, kontroluje instrukce a měří
rychlost. Spodní displej předvádí páčku/směry jako dotykový joystick a A/B jako
dotyková akční tlačítka. Výsledky ukládá do /3ds/java-3ds/symbian-probe.log.
Od 0.3.12 hlavní aplikace tento test rozbalí na SD při startu po aktualizaci CIA.
**Tento první test ještě nespouští Symbian ani SIS hry.** Zbývá port systému,
načítání aplikací, grafiky, zvuku a propojení s hlavní aplikací. Viz SYMBIAN.md.

## Předchozí změny 0.3.10

Nově: oprava priorit VM/Java/GC vláken a rekurzivního wait(), samostatné
pozastavení při HOME/zavření víka, omezené čekání při ukončení a úvodní načítání.
Po pěti sekundách neukončeného nativního volání CIA obnoví celý proces;
3DSX proces ukončí. Cizí živé vlákno se neuvolňuje do další hry. HOME a tento
obnovovací postup je stále potřeba ověřit přímo na konzoli.

DawnStar i Java Elder Scrolls Travels prošly v interaktivním PC testu až do
podzemí. A potvrzuje jejich nabídky, B se vrací, jméno zadává systémová
klávesnice. Přibyla azbuka. Gladiator 3D prošel do boje. Všechny tři dodané
varianty LOTR mají profil Portrait; obraz se vejde pomocí Scale to fit.

Aktualizace už během veřejného testování nepotřebují token. Z verze 0.3.9 bez
tokenu je ještě nutná jednorázová instalace nové CIA/QR, potom lze používat
Y → Wi-Fi updates → A. Repozitář zůstane veřejný, dokud nenapíšete, že testování
skončilo; poté se vrátí do soukromého režimu. Podrobnosti: UPDATES.md.

Nové Symbian/N-Gage hry jsou prověřené jako nativní aplikace, ale neběží v
Java VM. Chybí kompatibilní firmware telefonu i port emulátoru pro 3DS.
Podpora spuštění Symbianu zatím hotová není; viz SYMBIAN.md.

## Předchozí změny 0.3.9

Nová CIA zachovává původní aplikaci, hry, uložené pozice i nastavení na SD.
Menu hry otevře L + R nebo tlačítko na dotykovém displeji. START + SELECT
požádá o ukončení hry; požadavek nyní rovnou přerušuje VM, přípravu JARu,
čekání vláken a delší zvukové operace. V PC testu prošlo i opakované zrušení
spuštění a další start ve stejné aplikaci. Rychlost na konzoli je nutné ověřit.

Display má Scale to fit (zachová proporce, umí zmenšit vysoký obraz), Fill screen
a Native pixels. Před spuštěním lze zvolit Phone screen: Auto / Portrait /
Landscape. Dodaný LOTR potřebuje Portrait, i když název souboru říká 320 × 240.
Harem Adventures má profil Siemens 120 × 160 a upravené kódy ovládání; PC test
se dostal do první úrovně. Přibyla základní podpora formulářových prvků a
Siemens API. Elder Scrolls Travels a DawnStar se posunuly přes původní chyby
načítání. Úplné dohrání a rychlost těchto her zatím nejsou ověřené.

Sound tone v menu hry: Original / Soft / Warm. Soft je výchozí, tlumí ostré
výšky ve výsledném zvuku; ovlivňuje nyní i efekty. Original filtr vyřadí.

Micro3D má méně dělení v kreslicí smyčce: PC srovnání 10 000 trojúhelníků
mělo shodný obraz a čas 121 -> 106 ms. Nejde o zvýšení FPS celé hry o stejné
procento. Orcs & Elves 2 a Doom II RPG mají experimentální stereo podle
vzdáleností stěn. Podlaha a samostatné objekty nemají přesnou hloubku.

Banner používá navázání modelů z funkčního Faedrops 0.27. Odstranil jsem
časované protiootáčení, zachoval pětisekundovou animaci a nepřidal číslo verze.
Soubory prošly kontrolami, chování vlastní rotace v HOME Menu zatím musí
potvrdit konzole. Originální Blender soubor na ploše zůstal zachovaný.

Nejsou vyřešené všechny hry. Dodaný NFS The Run vyžaduje externí Reg/Util
knihovnu; aplikace tuto verzi odmítne s vysvětlením. Spore Creatures je stále
jen malý JAD bez herního JARu. Symbian emulace nebyla přidána. Další omezení
a konkrétní testy jsou v COMPATIBILITY.md a VALIDATION.md.

Předchozí funkce 0.3.8/0.3.7:



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
0.3.9 přidává zvláštní adaptér vzdáleností stěn pro tuto dodanou verzi. Nově uložené
Off zůstane zachované, starý výchozí stav Off se jednou převede na Auto.

V herním menu je CRT, v 0.3.8 rozšířené o výraznější barevné profily. Jemné filtry ovlivňují jen
hru a ukládají se pro titul. L+R nebo dotykové MENU otevře nastavení.

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
