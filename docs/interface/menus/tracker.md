
# Tracker

## Play

Begins playing the song from the current pattern. Resumes normal play if the
tracker was in step mode.

## Play from start

Begins playing the song from the first pattern in the order.

## Play at cursor

Begins playing the song from the cursor position.

## Step row

Begins playing the song from the cursor position in step mode. Step mode plays
a single row at a time.

## Stop

Stops music playback.

## Pattern repeat

Toggles pattern repeat mode. When pattern repeat mode is active, the tracker
will loop playback of the current pattern. Pattern repeat can be toggled at
any time.

## Record

Toggles record mode. When record mode is active, pattern data can be edited.

## Follow-mode

Toggles follow-mode. When follow-mode is active, the cursor row moves to the
current row being played out.

---

## Toggle channel output

Enables/Disables sound output for the cursor track.

## Solo

Solos/Unsolos the cursor track. Solo'ing a track mutes all tracks except for
the target track. 

---

## Reset volume

Resets the global volume scale to max on both terminals. The
[Jxy](../../tracker/effect-list.md#jxy-set-global-volume) effect is persistent
and is not reset when playing a new song. Use this action to reset it. See
[here](../../hardware/volume-control.md) for more details on the global volume
scale.

## Kill sound

Stops all sound output immediately. Differs from [Stop](#stop) in that sound
stops immediately by not letting the playback buffer drain.
