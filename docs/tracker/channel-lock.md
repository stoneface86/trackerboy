
# Channel locking

The channel lock mechanism is how the tracker reserves channels for music use
only. When a channel is **locked**, the tracker plays music by updating its sound
registers. A channel that is **unlocked** is not being updated by the tracker
and can be used for other purposes, such as sound effects.

By default, all channels are **locked** for music playback.

## Locked to Unlocked

When a channel is unlocked, its registers are cleared by writing zeros and its
panning bit flags are also reset in NR51. Thus the channel is given a blank
state when going from locked to unlocked. The tracker will also no longer update
this channel's registers when playing music.

## Unlocked to Locked

When an unlocked channel is locked again, its registers are updated with the
current state from the current song being played. If there is no song being
played, the registers are left untouched.

## Muting

This mechanism is intented to be used for sound effects, but is also used for
muting a channel. When a channel is muted, either by toggling the output or by
soloing a channel, the channel is unlocked. Since unlocking the channel silences
it by clearing its registers, this is essentially the same as muting it. To
unmute, the channel is just locked again.
