# Java3DS 0.3.2 (privátní alpha)

CIA: soubor java3ds.cia nainstaluj přes FBI. Java3DS se objeví v HOME Menu
s animovaným bannerem. Hry, MIDI i ukládání používají stejnou složku na SD.
QR: v FBI vyber Remote Install > Scan QR Code. Počítač musí běžet
a konzole musí být ve stejné domácí síti. Lokální QR platí 24 hodin;
Start-QR-Install.ps1 v instalační složce vytvoří nový a spustí server.

Pro verzi do Homebrew Launcheru: rozbal instalační ZIP do kořene SD karty a nahraď starý java_pocket.3dsx.
Složka /3ds/java-3ds se nemění; hry, nastavení a uložené pozice zůstanou zachované.
V Homebrew Launcheru se aplikace nově jmenuje Java3DS.

Nově: soukromé Wi-Fi aktualizace (Y v knihovně), mapování tlačítek pro každou hru,
skrytí spodního displeje v Controls, vycentrovaná klávesnice s reakcí na stisk,
paralax pozadí a animované Y2K načítání. Doplněno Nokia DirectGraphics pro
Might and Magic II a ošetřeno ukončování zaseknutých Java vláken.

Native obraz centruje. Režim 3DS screen u běžných her zvětšuje hotový obraz se
zachováním proporcí, takže mohou zůstat černé okraje; nemění interní rozlišení
hry. Širší kamera vyžaduje profil konkrétní hry — známý PoP Classic jej má.
Blades and Magic (K800i) stále vyžaduje nepodporovaný Mascot Capsule Micro3D.

Wi-Fi: vytvoř na GitHubu fine-grained token pouze pro Faefolk333/java3ds,
oprávnění Contents: Read-only. Samotný token ulož na jeden řádek do souboru
/3ds/java-3ds/github-token.txt na SD (prostý text, bez uvozovek a BOM).
Token neposílej do chatu ani nepřidávej do repozitáře. Podrobný postup je UPDATES.md.
Po startu se zkontroluje nová verze; A ji nainstaluje, B nabídku odloží.
Po instalaci aplikaci ukonči a znovu otevři. Verze 0.3.2 bude zatím aktuální;
Wi-Fi instalace se uplatní u příštího vydání. U verze 3DSX zůstává starší binárka jako .previous. CIA aktualizuje instalovaný titul.

Sestavení a testy proběhly na počítači. Wi-Fi, blikání displeje a skutečné hraní
ještě ověř na 3DS. Při problému zkopíruj runtime.log a output.log ze složky
/3ds/java-3ds před dalším spuštěním. Další podrobnosti: README.md a VALIDATION.md.
