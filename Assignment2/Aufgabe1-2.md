## Aufgabe 1
**a)**: 0x78010000 - Big Endian
**b)**: 
 ```perl
Header-Größe: 0x280000 = 40 Bytes (Big Endian)
Bildbreite: 0x0A000000 --LE--> 0x0000000A = 10 Pixel 
Bildhöhe: 0x0A000000 --LE--> 0x0000000A = 10 Pixel 
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
	`C_1.mbp`: 0x0C000000 **x** 0x0C000000
	`C_2.mbp`: 0x0C000000 **x** 0xF4FFFFFF
	-> 0x0 - 0x0C000000 = 0xF4FFFFFF (signed)

**d)**:

## Aufgabe 2
**a)**:
`A.bmp`: 0x0A000000 **x** 0x0A000000 --LE--> 0x0000000A **x** 0x0000000A = 10 **x** 10 Pixel
`B.bmp`: 0x09000000 **x** 0x09000000 --LE--> 0x00000009 **x** 0x00000009 = 9 **x** 9 Pixel