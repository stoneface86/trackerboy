# gbsynth

emu branch:

This branch is a rewrite of the synthesizer by emulating the hardware.

Individual components of the hardware are broken up into classes:
 * Mixer: mixes dac output from all channels, output is downsampled to a given
          sampling rate
 * Sequencer: Handles the timing for length counter, envelope and sweep
              functions for all channels
 * Channel: Base class for all sound channels, contains length counter
            (channel 3)
 * EnvChannel: subclass of channel adding the envelope function
               (channels 1, 2 and 4)
 * FreqChannel: utility class for channels with a frequency setting
                (channels 1, 2 and 3)
 * Sweep: the sweep function used only by channel 1
 * PulseChannel: channels 1 and 2, generates a rectangular waveform
 * WaveChannel: channel 3, generates a waveform from a 32 entry sample table
 * NoiseChannel: channel 4, generates noise
 * ChannelFile: just a POD of all 4 channels
 * Synth: main api class, container for the above classes
```
+- Synth --------------------------------------------+
|                                                    |
|      +--------> Sweep                              |
|      |            |                                |
|      |      +--> Ch1 -----+                        |
|      |      |             |                        |
| Sequencer --+--> Ch2 -----+--> Mixer -> Output     |
|             |             |                        |
|             +--> Ch3 -----+                        |
|             |             |                        |
|             +--> Ch4 -----+                        |
|                                                    |
+----------------------------------------------------+

```
WIP!

gbsynth is a C++ library for synthesizing sound based on the gameboy hardware.

Example
```C++
#define SAMPLING_RATE 44100

// gb sound 2: square wave with envelope, no sweep
gbsynth::SquareChannel ch(SAMPLING_RATE, false);
float buf[SAMPLING_RATE];
ch.setFrequency(440.0f)                 // Note: A4
ch.setDuty(gbsynth::DUTY_75)            // 75% duty
ch.setEnvLength(7);                     // step occurs every 7/64 seconds
ch.setEnvMode(gbsynth::ENV_ATTENUATE);  // envelope decreases
ch.setEnvStep(15);                      // initial envelope level (maximum)

// fill the sample buffer with 1 second of sound
ch.fill(buf, SAMPLING_RATE);

// play it, save to wav, etc

```
