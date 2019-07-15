# gbsynth

WIP!

gbsynth is a C++ library for synthesizing sound based on the gameboy hardware.
This synthesizer works by emulating the hardware, then downsampling the output
to a given sampling rate (ie 44100 Hz).

Individual components of the hardware are broken up into classes:
 * Mixer: mixes dac output from all channels to a 32-bit float
 * Sequencer: Handles the timing for length counter, envelope and sweep
              functions for all channels, as well as channels themselves
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

Example
```C++
#define SAMPLING_RATE 44100

gbsynth::Synth synth(SAMPLING_RATE);   // initialize synth with 44.1 KHz sampling rate

auto mixer = synth.getMixer();
mixer.setTermEnable(TERM_LEFT, true);  // enable S01 terminal (left channel)
mixer.setTermEnable(TERM_RIGHT, true); // enable S02 terminal (right channel)
mixer.setEnable(OUT_SOUND2_BOTH);      // output channel 2 to both terminals

gbsynth::PulseChannel &ch2 = synth.getChannels().ch2;
ch2.setFrequency(440.0f)                 // Note: A4
ch2.setDuty(gbsynth::DUTY_75)            // 75% duty
ch2.setEnvLength(7);                     // step occurs every 7/64 seconds
ch2.setEnvMode(gbsynth::ENV_ATTENUATE);  // envelope decreases
ch2.setEnvStep(15);                      // initial envelope level (maximum)
ch2.reset()                              // reset channel (sound init)

float bufLeft[SAMPLING_RATE];
float bufRight[SAMPLING_RATE];
// fill the sample buffers with 1 second of sound
synth.fill(bufLeft, bufRight, SAMPLING_RATE);

// play it, save to wav, etc
```

## Demos

The `demo` subdirectory contains example programs demonstrating the use of the
synthesizer and its output. To enable building these demo programs, set the
`ENABLE_DEMO` option ON when configuring. Portaudio is used for playback for
the demo programs so make sure the portaudio submodule is cloned before
configuring.
