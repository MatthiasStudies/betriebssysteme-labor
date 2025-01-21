

## Aufgabe 1
**a)**

HEEEEELLLLOOOOOOO -> 1H5E1L7O Text wird kürzer

ADADADADADAD -> 1A1D1A1D1A1D1A1D1A1D Text wird länger

**b)**

## Aufgabe 2

**a)**

`input & 1111000`

**b)**
```c
input = 01101010
left = input & 11110000
right = input & 00001111

left_shifted = left >> 4
right_shifted = right << 4

out = left_shifted | right_shifted

// Simpler:
out = (input << 4) | (input >> 4)
```

**c)**
Mit dem `&` (binary AND)
```c
in = 11010100
        
out = in & 00011000
```

**d)**
Mit dem `|` (binary OR)
```c
in = 10011011
        
out = in | 00000100
```



