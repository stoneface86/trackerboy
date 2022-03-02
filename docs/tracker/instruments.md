
# Instruments

Instruments in Trackerboy consists of sequences that modulate a certain
parameter once per frame. Sequences are essentially the same as what other
trackers/drivers call macros.

Instruments in Trackerboy are generic, and can be used on any channel.
Assigning a channel to an instrument is for organization and previewing purposes
only.

Instruments in a module are stored in a table, which can have up to 64 items.
The id of an instrument is its 6-bit index in this table, which is referenced
in track data.

Each instrument has a sequence for the following parameters:

 - [Arpeggio](#arpeggio)
 - [Panning](#panning)
 - [Pitch](#pitch)
 - [Timbre](#timbre)

Example sequences will be provided throughout this page like this: `[1 | 2 3]`.
The sequence data will be within the square brackets and a loop index (if
present) will be specified with the `|` character.


## Arpeggio

This sequence modulates the current note. Each value in the sequence is a relative
semitone offset that will change the current note being played.

Using this sequence with a loop index allows for more advanced arpeggios than
the [0xy](effect-list.md#0xy-arpeggio) effect.

The [0xy](effect-list.md#0xy-arpeggio) effect is equivalent to using a sequence
of `[| 0 x y]` where x and y are the effect parameters.

## Panning

This sequence modulates the channel's panning. It functions similarly to the
[I0x](effect-list.md#i0x-set-channel-panning) effect. Values in this sequence
should range from 0 to 3 for mute, left, right and middle, respectively.

Example uses:

 - A sequence of 1 value to just set the panning
 - Setting the panning to mute at the end of the sequence for frame timed cuts
 - Alternating between left and right for an echo effect


## Pitch

This sequence modulates the channel's frequency. Each channel has a counter
that is modified by the current instrument's pitch sequence. This counter value
is added when calculating the frequency. Each value in the sequence adds to
this counter.

For example consider this sequence: `[1 1 -1 -1]`
```
On frame 0, the counter is now 0 + 1 = 1 and the frequency is offset by 1
On frame 1, the counter is now 1 + 1 = 2 and the frequency is offset by 2
On frame 2, the counter is now 2 + -1 = 1 and the frequency is offset by 1
On frame 3 (and onward) the counter is now 1 + -1 = 0 and the frequency is offset by 0
```

If we add a loop index at 0, we have created a simple triangle vibrato effect
that alternates from 0 to 2.

To have a triangle vibrato that alternates from -2 to 2 use this:
`[-2 | 1 1 1 1 -1 -1 -1 -1]`

To add a downward slide do this: `[-2 | 1 1 1 1 -1 -1 -1 -1 -1]`

## Timbre

This sequence modulates the channel's timbre. Channel timbre is a channel-specific
setting. For pulse channels it is duty, for the wave channel it's the volume
and for the noise channel it's the step-width. Using a timbre sequence has a
similar effect to using the [V0x](effect-list.md#v0x-set-channel-timbre) effect.
Values in this sequence should range from 0 to 3.

Example uses:
 
 - Using this sequence on a pulse channel allows you to do PWM (pulse width
   modulation).
 - Use a 7-bit LFSR instead of the default 15-bit on the noise channel.
 - Can be used for frame timed cuts on wave channel by having a 0 at the end of
   the sequence.

## Loops

Sequences can be looped by specifying a loop index. The sequence will loop back
to this index when it reaches the end.
