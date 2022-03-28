
# Effect list

Below are all available effects. An effect is composed of a type and a
parameter. Some effects do not require a parameter. Note that the effects below
have similar syntax and function to Famitracker's effect set.

## 0xy - Arpeggio

 * x - 1st note
 * y - 2nd note

The arpeggio effect simulates a chord by alternating each note in the chord
every frame. The chord is made up of the current note and the effect's
parameters. Each parameter is the number of semitones from the current note.
The order of the arpeggio is the current note, calculated note from parameter
x followed by the calculated note from parameter y. If a calculated note
exceeds the highest, the highest will be used. Use `000` to turn off the effect.


## 1xx - Pitch slide up

 * `xx` - speed, in pitch units / frame

Modulates the channel's frequency upward using the given speed every frame.
Note that the perceived change in frequency is not linear. Use `100` to turn off
the effect. The effect will not increase the frequency past the channel's
maximum.

## 2xx - Pitch slide down

 * `xx` - speed, in pitch units / frame

Same as [1xx](#1xx-pitch-slide-up) but modulates the frequency downward.

## 3xx - Automatic Portamento

 * `xx` - speed, in pitch units / frame

Automatically slides to new notes with speed xx. Use `300` to disable the effect.

## 4xy - Square vibrato

 * `x` - speed, number of frames per oscillation in pitch
 * `y` - extent, in tune units

Modulates the channel's frequency using a square vibrato. The channel's
frequency alternates between +/- the extent (`y`). The speed, `x`, is the
number of frames needed for a single oscillation (1: fastest, F: slowest).
To disable, use an extent of 0 (`4x0`).

## 5xx - Set vibrato delay

 * `xx` - delay, in frames

The amount of frames to wait before starting the vibrato effect. By default,
all channels have a vibrato delay of 0, or instantaneous vibrato. The delay
starts on every note trigger.

## Bxx - Pattern goto

 * `xx` - the index of the pattern

Jumps to pattern xx in the order. If xx is >= the size of the order, the last
pattern is used. The current row finishes playing before jumping to the pattern.
If multiple effects of this type are used on the same row, the last one used is
the one that gets performed.

## C00 - Pattern halt

Stops playback of the song. Note that halting is immediate, the row the effect
lies on is not performed.

## Dxx - Pattern skip

 * `xx` - the row to start on

Skips to the next pattern in the order and starts on row xx. If the current
pattern is the last one in the order, the first pattern in the order is jumped
to. Similar to Bxx, the current row finishes playing before jumping.

## Fxx - Set speed

 * `xx` - the speed, in Q4.4 format

Changes the playback speed. The parameter is in Q4.4 format and must be in
range 1.0 and 15.0, inclusive. For more details on how speed works in
trackerboy, see the [speed](speed.md) page.

## Exx - Set envelope

 * `xx` - the envelope

Sets the channel's envelope setting. The type of this setting depends on the
channel the effect is used.

### For CH3:

Changes the current waveform in use. The parameter is the id of the wavefrom to
use.

### For CH1, CH2 and CH4:

Changes the channel's volume envelope. The volume envelope has the format - `xy`:

 - `x` - Volume level, 0-F
 - `y` - Fade speed, 0/8 no fade, 1-7 fade out, 9-F fade in

See [Channel envelope](../hardware/envelope.md) for more details about the
hardware.

## Gxx - Note delay
 
 * `xx` - number of frames to delay the note

Delays performance of the row by a given number of frames. If a non-empty row
is encountered before the delay expires, the delayed row is not performed.

## Hxx - Set sweep register

 * `xx` - value to set

Sets the sweep register (NR10) to the given value. Can only be used on CH1.
The sweep register is written, CH1 is retriggered and then the sweep register
is cleared. For an overview on the sweep register see
[Frequency sweep](../hardware/sweep.md).

## I0x - Set channel panning
 
 * `x` - panning to set

Sets the channel's panning to x. Valid values for x are 0 (mute), 1 (left),
2 (right) and 3 (middle). Using this effect may result in a "pop" sound due to
a change in the DC offset.

## Jxy - Set global volume
 
 * `x` - volume of left terminal (0-7)
 * `y` - volume of right terminal (0-7)

Sets the global volume scale. This effect writes to the NR50 register. Note
that a volume of 0 is not mute. By default, both terminals have a volume of 7.

**Warning:** This effects the volume of both music and SFX.

## L00 - Lock channel (music priority)

This effect is currently not implemented.


## Pxx - Fine tuning
 * `xx` - tune offset in pitch units

Sets a constant pitch offset for the track. The parameter is in biased
representation with K = 0x80. A parameter of 0x80 means the track is in tune
or has no pitch offset. A parameter of 81 means that the track is pitch
adjusted by +1. By default, all tracks have a tune setting of 0x80.

## Qxy - Note slide up
 
 * `x` - slide speed, in pitch units / frame
 * `y` - semitones from the current note to slide to

Sets a triggered note slide for the current track. The track's frequency will
slide towards the frequency of the note calculated from adding y semitones to
the current note. The x parameter is the speed of the slide, in pitch units per
frame + 1 (0 is the slowest speed and F is the fastest). If the targeted note
exceeds the highest possible one (B-8), then the highest will be used instead.

## Rxy - Note slide down
 
 * `x` - slide speed, in pitch units / frame
 * `y` - semitones from the current note to slide to

Same as [Qxy](#Qxy-note-slide-up), except the targeted note is the current
note minus y semitones. If the targeted note is less than the lowest, the
lowest is used instead.

## Sxx - Delayed note cut
 
 * `xx` - delay, in frames

Cuts the note in a given number of frames. Can be combined with
[Gxx](#Gxx-note-delay).

## Txx - Play sound effect

 * `xx` - sound effect id to play

This effect is not currently implemented.

## V0x - Set channel timbre

 * `x` - timbre to set

Sets the channel's timbre. Timbre is a setting specific to a channel.

### For CH1, CH2:

Duty setting:

 * `0` -  12.5% Duty
 * `1` - 25% Duty
 * `2` - 50% Duty
 * `3` - 75% Duty (default)

### For CH3:

Wave volume:

 * `0` - Mute
 * `1` - 25% volume
 * `2` - 50% volume
 * `3` - 100% volume (default)

### For CH4:

Step-width of the LFSR:

 * `0` - 15-bit LFSR (default)
 * `1` - 7-bit LFSR
