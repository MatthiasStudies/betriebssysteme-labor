## Aufgabe 1
**a)**: 0x78010000 - Little Endian
**b)**:
 ```perl
Header-Größe: 0x280000 = 40 Bytes (Little Endian)
Bildbreite: 0x0A000000 --LE--> 0x0000000A = 12 Pixel
Bildhöhe: 0x0A000000 --LE--> 0x0000000A = 12 Pixel
Anzahl Farbebenen: 0x0100 --LE--> 0x0001 = 1 Farbebene
Bits pro Pixel: 0x1800 --LE--> 0x0018 = 24 Bit pro Pixel
Kompressionsmethode: 0x00000000 --LE--> 0x00000000 = keine Kompression (i guess)
Bildgröße: 0x42010000 --LE--> 0x00000142 = 322 Bytes
Horizontale Auflösung: 0xC21E0000 --LE--> 0x00001EC2 = 7874 DPI / PPM
Vertikale Auflösung: 0xC21E0000 --LE--> 0x00001EC2 = 7874 DPI / PPM
Anzahl der Farben in Palette: 0x00000000 --LE--> 0x00000000
Anzahl wichtiger Farben: 0x00000000 --LE--> 0x00000000
 ```

**c)**:
	`C_1.bmp`: 0x0C000000 **x** 0x0C000000
	`C_2.bmp`: 0x0C000000 **x** 0xF4FFFFFF
	-> 0x0 - 0x0C000000 = 0xF4FFFFFF (signed)

**d)**:
- `C_1.bmp` hat `0x0100` Bits pro Pixel (die anderen `0x1800`)
- `C_1.bmp` hat Bildgröße von `0x32000000` (die andere `0xB2010000`)
- `C_1.bmp` hat horizontale / vertikale Aflösung von `0x120B0000` (die anderen `0xC2E10000`)

## Aufgabe 2
**a)**:
- `A.bmp`: 0x0A000000 **x** 0x0A000000 --LE--> 0x0000000A **x** 0x0000000A = 10 **x** 10 Pixel
- `B.bmp`: 0x09000000 **x** 0x09000000 --LE--> 0x00000009 **x** 0x00000009 = 9 **x** 9 Pixel
- In BGR Format
- Bild wird Bottom up gelesen
- Padding für 4byte boundery

**b)**:
Idee: Da der Bildbereich bei `C_2.bmp` eine negative Höhe hat, müssen die Pixel in umgekehrter Reihenfolge angegeben werden.

**c)**:
- Offset 62 (0x3E) Bytes -> Farbpallette existiert
- 1 Bit pro Pixel
- Farbpalette mit 2 Farben
- Jede Farbe ist 4 Byte groß
- Jede Zeile 4 Byte (inkl. 20 Bit Padding), mit 1 = Grün und 0 = Schwarz
