
# Speed/Tempo

Speed refers to the rate at which rows are performed by the tracker. It can
also be thought as the tempo of the song. This page will explain how speed
works in Trackerboy.

The playback speed of a song is determined by the global speed setting. This 
setting is initialized by the song's speed setting and can be changed during
playback with the [Fxx](effect-list.md#fxx-set-speed) effect. This
setting determines the number of frames each row gets when playing. Fixed point
is the format of this setting with 4 bits for the integral and 4 bits for the
fraction (Q4.4). For more details on fixed point see [Q-format](https://en.wikipedia.org/wiki/Q_(number_format)).
In other words, the speed setting is composed of two hex nibbles. The first
nibble being the integral, or the whole number of frames each row gets. The
second nibble is the fraction, which determines how often some rows get an
extra frame.

Valid speeds range from 1.0 to 15.0 (0x10 to 0xF0), inclusive. Speed 1.0,
unit speed, is the fastest and 15.0 is the slowest. The default speed for new
songs is 6.0 (0x60).

### Whole-number speed

For speeds where each row gets the same number of frames, use `x0` where `x` is
a hex digit 1-F being the number of frames.

### Fractional speed

When the fractional component of the speed is nonzero, some rows will play for
an extra frame. The ratio of rows that get an extra is determined by the
fraction. For example, a fractional component of 1/2 (`x8`), means that 1 of
every 2 rows will get an extra frame. Whereas a component of 1/4 (`x4`) means
that 1 of every 4 rows will get an extra frame.

## Tempo

Using the speed setting, we can determine what the tempo of the song is. We can
also determine a speed setting for any given tempo.

### Speed to tempo calculation

We can convert a speed to a tempo using the following formula:

$$tempo = {{60 * framerate} \over {speed * rpb}}$$

#### Definitions

 * framerate - the rate the tracker is updated, typically the vblank interval (59.7 Hz)
 * tempo - the pace of a song, measured in beats per min (bpm)
 * rpb - acronym for rows per beat. This value determines the number of rows a
   single beat requires.
 * speed - the number of ticks a row is performed for, measured in frames per
   row (fpr)

#### Examples

When \(rpb = 4\), \(speed = 2.5\), \(framerate = 59.7\)

$$ tempo = {{60 * 59.7} \over {2.5 * 4}} = {3582 \over 10} = 358.2 bpm $$


### Tempo to speed calculation

We can also estimate a speed from any given tempo using a similar formula:

$$speed = {{60 * framerate} \over {tempo * rpb}}$$

#### Examples

When \(rpb = 8\), \(tempo = 170\), \(framerate = 59.7\)

$$ speed = { {60 * 59.7} \over {8 * 170}} = { 3582 \over 1360 } = 2.634 \approx 2.625 {fpr}$$

Note that we cannot represent 2.634 using Q4.4 so the closest value is 2.625 or
0x2A. If we calculate the tempo using this speed value we get 170.571 bpm. Some
tempos, like this example, cannot be accurately represented by our speed setting.

## Tempo calculator

These calculations are provided as an example. When using Trackerboy you do not
need to do these calculations yourself, simply use the
[Tempo calculator](../interface/tempo-calculator.md) utility
(Song > Tempo calculator...) instead.
