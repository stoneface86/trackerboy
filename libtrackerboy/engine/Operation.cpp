
#include "trackerboy/engine/Operation.hpp"
#include "trackerboy/note.hpp"

namespace trackerboy {

Operation::Operation() :
    patternCommand(PatternCommand::none),
    patternCommandParam(0),
    speed(0),
    halt(false),
    note(),
    instrument(),
    delay(0),
    duration(),
    envelope(),
    timbre(),
    panning(),
    sweep(),
    modulationType(FrequencyMod::none),
    modulationParam(0),
    vibrato(),
    vibratoDelay(),
    tune()
{
}


Operation::Operation(TrackRow const& row) :
    patternCommand(PatternCommand::none),
    patternCommandParam(0),
    speed(0),
    halt(false),
    note(),
    instrument(),
    delay(0),
    duration(),
    envelope(),
    timbre(),
    panning(),
    sweep(),
    modulationType(FrequencyMod::none),
    modulationParam(0),
    vibrato(),
    vibratoDelay(),
    tune()
{

    // note column
    note = row.queryNote();
    if (note && *note == NOTE_CUT) {
        // NOTE_CUT behaves exactly the same as the S00 effect
        // this also makes the Sxx effect have higher priority unless we process the note after effects
        // --  .. ... ... ... same as: ... .. S00 ... ...
        // --  .. S03 ... ... => the row will cut in 3 frames
        note.reset();
        duration = (uint8_t)0;
    }

    // instrument column
    instrument = row.queryInstrument();

    // effects
    for (size_t i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
        auto effect = row.queryEffect(i);
        if (effect) {
            auto param = effect->param;
            switch (effect->type) {
                case trackerboy::EffectType::patternGoto:
                    patternCommand = PatternCommand::jump;
                    patternCommandParam = param;
                    break;
                case trackerboy::EffectType::patternHalt:
                    halt = true;
                    break;
                case trackerboy::EffectType::patternSkip:
                    patternCommand = PatternCommand::next;
                    break;
                case trackerboy::EffectType::setTempo:
                    if (param >= SPEED_MIN && param <= SPEED_MAX) {
                        speed = param;
                    }
                    break;
                case trackerboy::EffectType::sfx:
                    // TBD
                    break;
                case trackerboy::EffectType::setEnvelope:
                    envelope = param;
                    break;
                case trackerboy::EffectType::setTimbre:
                    timbre = param;
                    break;
                case trackerboy::EffectType::setPanning:
                    switch (param) {
                        case 0x00:
                            panning = Panning::mute;
                            break;
                        case 0x01:
                            panning = Panning::right;
                            break;
                        case 0x10:
                            panning = Panning::left;
                            break;
                        case 0x11:
                            panning = Panning::middle;
                            break;
                        default:
                            break;
                    }
                    break;
                case trackerboy::EffectType::setSweep:
                    sweep = param;
                    break;
                case trackerboy::EffectType::delayedCut:
                    duration = param;
                    break;
                case trackerboy::EffectType::delayedNote:
                    delay = param;
                    break;
                case trackerboy::EffectType::lock:
                    // TBD
                    break;
                case trackerboy::EffectType::arpeggio:
                    modulationType = FrequencyMod::arpeggio;
                    modulationParam = param;
                    break;
                case trackerboy::EffectType::pitchUp:
                    modulationType = FrequencyMod::pitchSlideUp;
                    modulationParam = param;
                    break;
                case trackerboy::EffectType::pitchDown:
                    modulationType = FrequencyMod::pitchSlideDown;
                    modulationParam = param;
                    break;
                case trackerboy::EffectType::autoPortamento:
                    modulationType = FrequencyMod::portamento;
                    modulationParam = param;
                    break;
                case trackerboy::EffectType::vibrato:
                    vibrato = param;
                    break;
                case trackerboy::EffectType::vibratoDelay:
                    vibratoDelay = param;
                    break;
                case trackerboy::EffectType::tuning:
                    tune = param;
                    break;
                case trackerboy::EffectType::noteSlideUp:
                    modulationType = FrequencyMod::noteSlideUp;
                    modulationParam = param;
                    break;
                case trackerboy::EffectType::noteSlideDown:
                    modulationType = FrequencyMod::noteSlideDown;
                    modulationParam = param;
                    break;
            }
        }
    }

}

}
