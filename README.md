# gbsynth

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
