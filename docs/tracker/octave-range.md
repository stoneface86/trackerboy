
# Octave range and notes

Notes in Trackerboy range from octaves 2 to 8. [Scientific pitch notation (SPN)](https://en.wikipedia.org/wiki/Scientific_pitch_notation)
is used, with C-2 being approximately 65.4 Hz (concert pitch). Valid notes
range from C-2 to B-8, inclusive, for channels 1-3. Valid notes on channel 4
range from C-2 to B-6, inclusive. Attempting to use a note outside this range
will result in the note being clamped to its respective boundary.

## Channel frequency

Each channel has a frequency setting. Frequency in channels 1-3 is an 11-bit
number (0-2047) with 0 being the lowest and 2047 being the highest. Frequency
in channel 4 is a byte, with a specific format explained [here](../hardware/noise-channel.md).
Notes in Trackerboy are mapped to specific frequencies using a table lookup.
See below for the tables used.

## Channel 1-3 Frequency table

| Octave | C     | C#    | D     | D#    | E     | F     | F#    | G     | G#    | A     | A#    | B     |
|--------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
| 2      | 0x02C | 0x09D | 0x107 | 0x16B | 0x1CA | 0x223 | 0x277 | 0x2C7 | 0x312 | 0x358 | 0x39B | 0x3DA |
| 3      | 0x416 | 0x44E | 0x483 | 0x4B5 | 0x4E5 | 0x511 | 0x53C | 0x563 | 0x589 | 0x5AC | 0x5CE | 0x5ED |
| 4      | 0x60B | 0x627 | 0x642 | 0x65B | 0x672 | 0x689 | 0x69E | 0x6B2 | 0x6C4 | 0x6D6 | 0x6E7 | 0x6F7 |
| 5      | 0x706 | 0x714 | 0x721 | 0x72D | 0x739 | 0x744 | 0x74F | 0x759 | 0x762 | 0x76B | 0x773 | 0x77B |
| 6      | 0x783 | 0x78A | 0x790 | 0x797 | 0x79D | 0x7A2 | 0x7A7 | 0x7AC | 0x7B1 | 0x7B6 | 0x7BA | 0x7BE |
| 7      | 0x7C1 | 0x7C5 | 0x7C8 | 0x7CB | 0x7CE | 0x7D1 | 0x7D4 | 0x7D6 | 0x7D9 | 0x7DB | 0x7DD | 0x7DF |
| 8      | 0x7E1 | 0x7E2 | 0x7E4 | 0x7E6 | 0x7E7 | 0x7E9 | 0x7EA | 0x7EB | 0x7EC | 0x7ED | 0x7EE | 0x7EF |

## Channel 1-3 Actual frequency

| Note | Frequency (Hz) |
|------|----------------|
|  C-2 |          65.41 |
|  C#2 |          69.31 |
|  D-2 |          73.43 |
|  D#2 |          77.79 |
|  E-2 |          82.38 |
|  F-2 |          87.32 |
|  F#2 |          92.50 |
|  G-2 |          98.03 |
|  G#2 |         103.86 |
|  A-2 |         109.96 |
|  A#2 |         116.51 |
|  B-2 |         123.42 |
|  C-3 |         130.81 |
|  C#3 |         138.55 |
|  D-3 |         146.78 |
|  D#3 |         155.48 |
|  E-3 |         164.87 |
|  F-3 |         174.53 |
|  F#3 |         184.87 |
|  G-3 |         195.92 |
|  G#3 |         207.72 |
|  A-3 |         219.92 |
|  A#3 |         233.22 |
|  B-3 |         246.84 |
|  C-4 |         261.62 |
|  C#4 |         277.11 |
|  D-4 |         293.88 |
|  D#4 |         311.33 |
|  E-4 |         329.33 |
|  F-4 |         349.53 |
|  F#4 |         370.26 |
|  G-4 |         392.43 |
|  G#4 |         414.78 |
|  A-4 |         439.84 |
|  A#4 |         466.45 |
|  B-4 |         494.61 |
|  C-5 |         524.29 |
|  C#5 |         555.39 |
|  D-5 |         587.77 |
|  D#5 |         621.19 |
|  E-5 |         658.65 |
|  F-5 |         697.19 |
|  F#5 |         740.52 |
|  G-5 |         784.86 |
|  G#5 |         829.57 |
|  A-5 |         879.68 |
|  A#5 |         929.59 |
|  B-5 |         985.50 |
|  C-6 |        1048.58 |
|  C#6 |        1110.78 |
|  D-6 |        1170.29 |
|  D#6 |        1248.30 |
|  E-6 |        1323.96 |
|  F-6 |        1394.38 |
|  F#6 |        1472.72 |
|  G-6 |        1560.38 |
|  G#6 |        1659.14 |
|  A-6 |        1771.24 |
|  A#6 |        1872.46 |
|  B-6 |        1985.94 |
|  C-7 |        2080.51 |
|  C#7 |        2221.56 |
|  D-7 |        2340.57 |
|  D#7 |        2473.06 |
|  E-7 |        2621.44 |
|  F-7 |        2788.77 |
|  F#7 |        2978.91 |
|  G-7 |        3120.76 |
|  G#7 |        3360.82 |
|  A-7 |        3542.49 |
|  A#7 |        3744.91 |
|  B-7 |        3971.88 |
|  C-8 |        4228.13 |
|  C#8 |        4369.07 |
|  D-8 |        4681.14 |
|  D#8 |        5041.23 |
|  E-8 |        5242.88 |
|  F-8 |        5698.78 |
|  F#8 |        5957.82 |
|  G-8 |        6241.52 |
|  G#8 |        6553.60 |
|  A-8 |        6898.53 |
|  A#8 |        7281.78 |
|  B-8 |        7710.12 |

## Channel 4 Frequency table

| Octave | C    | C#   | D    | D#   | E    | F    | F#   | G    | G#   | A    | A#   | B    |
|--------|------|------|------|------|------|------|------|------|------|------|------|------|
| 2      | 0xD7 | 0xD6 | 0xD5 | 0xD4 | 0xC7 | 0xC6 | 0xC5 | 0xC4 | 0xB7 | 0xB6 | 0xB5 | 0xB4 | 
| 3      | 0xA7 | 0xA6 | 0xA5 | 0xA4 | 0x97 | 0x96 | 0x95 | 0x94 | 0x87 | 0x86 | 0x85 | 0x84 | 
| 4      | 0x77 | 0x76 | 0x75 | 0x74 | 0x67 | 0x66 | 0x65 | 0x64 | 0x57 | 0x56 | 0x55 | 0x54 | 
| 5      | 0x47 | 0x46 | 0x45 | 0x44 | 0x37 | 0x36 | 0x35 | 0x34 | 0x27 | 0x26 | 0x25 | 0x24 | 
| 6      | 0x17 | 0x16 | 0x15 | 0x14 | 0x07 | 0x06 | 0x05 | 0x04 | 0x03 | 0x02 | 0x01 | 0x00 |
