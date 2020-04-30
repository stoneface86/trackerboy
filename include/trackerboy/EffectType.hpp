
#pragma once

#include <cstdint>


namespace trackerboy {

enum class EffectType : uint8_t {
    // A * indicates the effect is continuous and must be turned off (ie 400)
    arpeggio,           // * 1 2 3   0xy arpeggio with semi tones x and y
    pitchUp,            // * 1 2 3   1xx pitch slide up
    pitchDown,          // * 1 2 3   2xx pitch slide down
    autoPortamento,     // * 1 2 3   3xx automatic portamento
    vibrato,            // * 1 2 3   4xy vibrato
    patternGoto,        //   1 2 3 4 Bxx begin playing given pattern immediately
    patternHalt,        //   1 2 3 4 C00 stop playing
    patternSkip,        //   1 2 3 4 D00 begin playing next pattern immediately
    setEnvelope,        //   1 2 3 4 Exx set the persistent envelope/wave id setting
    setTempo,           //   1 2 3 4 Fxx set the tempo
    setSweep,           //   1       Hxx set the persistent sweep setting (CH1 only)
    setPanning,         //   1 2 3 4 Ixy set channel panning setting
    runMacro,           //   1 2 3 4 Mxx run the given macro
    stopMacro,          //   1 2 3 4 N00 stop the current running macro
    tuning,             //   1 2 3   Pxx fine tuning
    noteSlideUp,        // * 1 2 3   Qxy note slide up
    noteSlideDown,      // * 1 2 3   Rxy note slide down
    sfx,                // * 1 2 3 4 Sxx play sound effect
    sfxStop,            //   1 2 3 4 T0x stop the sound effect playing on the given channels
    setTimbre           //   1 2 3   Vxx set persistent duty/wave volume setting
};

}
