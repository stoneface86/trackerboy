
#pragma once

#include <cstdint>

namespace trackerboy {

constexpr uint8_t EFFECT_CATEGORY_PATTERN = 0x00;
constexpr uint8_t EFFECT_CATEGORY_TRACK = 0x40;
constexpr uint8_t EFFECT_CATEGORY_FREQ = 0x80;

enum class EffectType : uint8_t {

    // A * indicates the effect is continuous and must be turned off (ie 400)

    // pattern effect (bits 6-7 = 00)
    patternGoto = EFFECT_CATEGORY_PATTERN,  //   1 2 3 4 Bxx begin playing given pattern immediately
    patternHalt,                            //   1 2 3 4 C00 stop playing
    patternSkip,                            //   1 2 3 4 D00 begin playing next pattern immediately
    setTempo,                               //   1 2 3 4 Fxx set the tempo
    sfx,                                    // * 1 2 3 4 Txx play sound effect
    
    // track effect (bits 6-7 = 01)

    setEnvelope = EFFECT_CATEGORY_TRACK,    //   1 2 3 4 Exx set the persistent envelope/wave id setting
    setTimbre,                              //   1 2 3 4 Vxx set persistent duty/wave volume setting
    setPanning,                             //   1 2 3 4 Ixy set channel panning setting
    setSweep,                               //   1       Hxx set the persistent sweep setting (CH1 only)
    delayedCut,                             //   1 2 3 4 Sxx note cut delayed by xx frames
    delayedNote,                            //   1 2 3 4 Gxx note trigger delayed by xx frames
    lock,                                   //   1 2 3 4 L00 (lock) stop the sound effect on the current channel


    // frequency effect (bits 6-7 = 10)
    arpeggio = EFFECT_CATEGORY_FREQ,        // * 1 2 3   0xy arpeggio with semi tones x and y
    pitchUp,                                // * 1 2 3   1xx pitch slide up
    pitchDown,                              // * 1 2 3   2xx pitch slide down
    autoPortamento,                         // * 1 2 3   3xx automatic portamento
    vibrato,                                // * 1 2 3   4xy vibrato
    vibratoDelay,                           //   1 2 3   5xx delay vibrato xx frames on note trigger
    tuning,                                 //   1 2 3   Pxx fine tuning
    noteSlideUp,                            // * 1 2 3   Qxy note slide up
    noteSlideDown                           // * 1 2 3   Rxy note slide down
    
};

}
