
# Glossary

This page contains some terminology used throughout the manual.

## Hardware

### APU

The APU, or audio processing unit, is the hardware component that handles
sound output.

### Channel

A channel is a part of the APU that generates a waveform. The Game Boy has 4
channels for use, two pulse channels, one waveform channel and one noise
channel. Channels are often abbrievated as `CHx` where x is the channel number
from 1-4.

### Pulse channel

The pulse channel is a channel that generates a rectangular waveform. Channels
1 and 2 are pulse channels. Each pulse channel has 4 different duty settings.
See the [pulse channel](../hardware/pulse-channel.md) page for more details.

### Wave channel

The wave channel is a channel that generates a waveform from a customizable
32-step waveform or wavetable. Channel 3 is a wave channel.
See the [wave channel](../hardware/wave-channel.md) page for more details.

### Noise channel

The noise channel is a channel that generates white noise via a linear feedback
shift register (LFSR). Channel 4 is a noise channel.
See the [noise channel](../hardware/noise-channel.md) page for more details.

### Envelope

The envelope is a hardware volume envelope that handles linear volume sweeps.
Only channels 1, 2 and 4 have an envelope. These channels are also referred to
as envelope channels. See the [envelope](../hardware/envelope.md) page for more
details.

### Sweep

The sweep is a hardware frequency sweep for channel 1. When activated the sweep
modifies the channel's frequency by increasing or decreasing it periodically.
See the [sweep](../hardware/sweep.md) page for more details.

### DAC

A DAC is a digital to analog converter. Each channel has a DAC which converts
the digital signal created by the channel to an analog one which is mixed and
outputted to the speakers.

### Mixer

The mixer combines all signals from each channel's DAC and sends it to the
speakers. The mixer contains a global volume setting for each terminal, as well
as switches for channel DAC output to each terminal.

### Terminal

Output to be sent to a speaker. The Game Boy has stereo sound so there are two
terminals, one for the left speaker and one for the right speaker. If
headphones are not used, both terminals are combined into a mono output and is
sent to the Game Boy's builtin speaker.

### Register

Register refers to the memory mapped hardware registers of the APU. The APU is
controlled by writing to these registers. Registers are referenced by the
following naming scheme: `NRxy` where `x` is the channel (or 5 for the control
registers) and `y` is the register number.

## Playback

### Driver

A sound driver handles music and sound effect playback on a Game Boy by
periodically updating its APU registers.

### Engine

The engine is responsible for playing music and in the future, sound effects.
The engine behaves similarly to a driver, but differs in that it runs on your
PC and sends register writes to an APU emulator.

### Halt

A halt is an operation that ends playback of a song. Halts can be performed by
the song via the `C00` effect, or forcibly by the application.

### Locked/Unlocked

A channel is *locked* if it has music playing on it, otherwise it is *unlocked*
and can be used for other purposes (typically sound effects). See the
[Channel lock mechanism](channel-lock.md) page for more details.

### Music Runtime

Used internally by the tracker's engine to manage the playback of a song.
The runtime only plays one song indefinitely or until it halts. A new runtime
is constructed for each song that is played or restarted.

### Speed

Tracker playback speed in units of frames/row, in fixed point format Q4.4. See
the [Speed](speed.md) page for more details.

## Data

### Pattern

A pattern is a collective of 4 tracks, one for each channel.

### Order

An order is the layout of a song, as a list of order rows. Each row in the
order specifies a pattern, which the song will play from first to last. When
the last order has finished playing, the song loops back to first one.

### Order Row

A row in the order. An order row contains four track ids, one for each channel.

### Row

A row is a singular piece of music data. A track row is a single row in a Track,
whereas a pattern row is a single row in a pattern, or a collective of 4 track
rows.

### Column

A cell or part of a track row. There are three types of columns:

 1. Note
 2. Instrument
 3. Effect

### Note

A note is a specific frequency setting. A note is also a type of column that
indicates the row will start a note. A channel can only be started by using a
note column.

There are two types of notes: tone and noise. A tone note represents a
frequency that is playable on the pulse and wave channels. Tone notes range
from C-2 to B-8. Concert pitch is used with some notes being off key due to
hardware limitations. A noise note represents a frequency setting that is only
playable on the noise channel. Noise notes range from C-2 to B-6, and each note
is used as an index in a lookup table that results in a unique frequency setting
for the noise channel. Noise frequencies are sorted from the lowest at C-2 to
the highest at B-6.

For more details on these notes see the [Octave Range](octave-range.md) page.

### Instrument

An instrument is a tool for manipulating the feel, volume and overall sound of
a channel when a note is played. An instrument accomplishes this by modifying
the state of channel using sequences. See the [Instrument](instruments.md) page
for more details on their operation.

### Waveform

A waveform is a 16-byte sequence of nibbles that is used for the wave channel's
wavetable. The upper nibble of the first byte is the first sample in the wavetable

### Sequence

A sequence is an array of bytes that are used by instruments to manipulate a
channel's parameter. They are similar in function to what other trackers call
macros. Sequences can also have a loop index, which will cause the instrument to
loop back to that index when the end of the sequence is reached.

