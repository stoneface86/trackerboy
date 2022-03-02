
# Frequently asked questions

The following are some answers to some common questions about Trackerboy.

## General

 * *How do I get started?*

You could create some instruments, make a couple for each channel to explore
the hardware's capabilities. Or you can dive right in and start entering notes
on the pattern editor. Unlike some other trackers, instruments are 100%
optional (but recommended!) to make music.

 * *Is there a macOS release?*

Currently no, I do not provide macOS releases as I cannot test them. You can
build from source or use the windows release via WINE. If you do use WINE,
change the audio API in the Sound / Midi configuration to DirectSound or WinMM
as the WASAPI backend does not work in WINE.

 * *A button I want to use is grayed out, what do I do?*

The feature you are attempting to use is most likely not implemented at this
time, you will have to wait for a newer release.

## Editing

 * *How do I enter notes?*

Focus on the pattern editor and hit Space to toggle record mode, the cursor row
will change color to red when in record mode. You can now enter notes using
your keyboard.

 * *How do I play and loop a single pattern?*

Select the pattern you want to play and enable repeat-mode (Tracker > Pattern
repeat), then play the song.

 * *I don't have a QWERTY keyboard, can I change the default layout?*

Yes, open the configuration (File > Configuration) and select the Keyboard tab.
You can choose your layout in the combobox or setup a custom one if yours is
not provided.

 * *Why are the notes on CH3 one octave lower?*

CH3's frequency range is one octave lower than the pulse channels if you have a
single period waveform. Notes in trackerboy are based on the octave range of
the pulse channels. See [Wave channel](hardware/wave-channel.md) for more details.

## Troubleshooting

 * *Sound is stuttering or clicking.*

The buffer length is too short, and underruns are occuring. Check your sound
settings, gradually increase the buffer size until the problem is fixed. Also
make sure that the buffer size is at least twice the size of the period.

 * *There's no sound.*

Try a different device and/or api in the sound settings.

 * *My device disconnected and the tracker won't stop playing.*

Kill sound (Tracker > Kill Sound) and select a new device in the audio settings
or the same device after reconnecting it.
