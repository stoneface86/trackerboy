
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/note.hpp"

namespace trackerboy {

MusicRuntime::MusicRuntime(RuntimeContext rc, Song &song, uint8_t orderNo, uint8_t patternRow) :
    mRc(rc),
    mSong(song),
    mOrderCounter(orderNo),
    mLastOrder(static_cast<uint8_t>(song.orders().size() - 1)),
    mRowsPerTrack(song.patterns().rowSize()),
    mCommand(PatternCommand::none),
    mCommandParam(0),
    mEnvelope{ 0xF0, 0xF0, 0x00, 0xF0 },
    mTimbre{ 0, 0, 3, 0 },
    mPanning(0xFF),
    mPanningMask(0x00),
    mNoteDelay(0),
    mFlags(FLAGS_DEFAULT)
{
    mCursor.setPattern(song.getPattern(orderNo), patternRow);
    mTimer.setPeriod(song.speed());
}

template <ChType ch>
bool MusicRuntime::setRows() {

    constexpr int chint = static_cast<int>(ch);
    //bool const locked = !!((FLAGS_LOCK1 << chint) & mFlags);

    bool freqEffectSet = false;
    if constexpr (ch == ChType::ch4) {
        // we don't need this variable for CH4
        (void)freqEffectSet;
    }

    mNoteDelay = 0; // default note delay is no delay

    TrackRow &row = mCursor.get<ch>();

    for (size_t i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
        if (!!(row.flags & (TrackRow::COLUMN_EFFECT1 << i))) {
            Effect effect = row.effects[i];
            uint8_t category = static_cast<uint8_t>(effect.type) & 0xC0;
            switch (category) {
                case EFFECT_CATEGORY_PATTERN:
                    if (processPatternEffect(effect)) {
                        return true;
                    }
                    break;
                case EFFECT_CATEGORY_TRACK:
                    processTrackEffect<ch>(effect);
                    break;
                case EFFECT_CATEGORY_FREQ:
                    if constexpr (ch != ChType::ch4) {
                        freqEffectSet = processFreqEffect(effect, mFc[chint]);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    if (!!(row.flags & TrackRow::COLUMN_NOTE)) {
        mNc[chint].noteTrigger(row.note, mNoteDelay);
    } else {
        // a frequency effect was set 
        if constexpr (ch != ChType::ch4) {
            if (freqEffectSet) {
                mFc[chint].apply();
            }
        }
    }

    if constexpr (ch != ChType::ch4) {
        return setRows<static_cast<ChType>(chint + 1)>();
    } else {
        return false;
    }

}

bool MusicRuntime::step() {
    if (mFlags & FLAGS_HALTED) {
        return true;
    }

    // if timer is active, we are starting a new row
    if (mTimer.active()) {

        // change the current pattern if needed
        switch (mCommand) {
            case PatternCommand::none:
                break;
            case PatternCommand::next:
                if (++mOrderCounter > mLastOrder) {
                    // loop back to the first pattern
                    mOrderCounter = 0;
                }
                mCursor.setPattern(mSong.getPattern(mOrderCounter), mCommandParam);
                mCommand = PatternCommand::none;
                break;
            case PatternCommand::jump:
                mCursor.setPattern(mSong.getPattern(mCommandParam));
                mCommand = PatternCommand::none;
                break;
        }

        // apply the current row to runtime components
        bool halt = setRows();

        if (halt) {
            mFlags |= FLAGS_HALTED;
            return true;
        }

    }

    update<ChType::ch1>();
    update<ChType::ch2>();
    update<ChType::ch3>();
    update<ChType::ch4>();


    // update panning
    if (!!(mFlags & FLAGS_PANNING)) {

        // determine music panning
        uint8_t panning = mPanning & mPanningMask;

        //uint8_t lockbits = mFlags & 0xF;
        //if (lockbits != 0xF) {
        //    // when one or more channels are unlocked:
        //    // NR51 <- ((y & z) & w) | (x & ~w)
        //    // where
        //    //  w: channel lock bits
        //    //  x: current value of NR51
        //    //  y: mPanning
        //    //  z: mPanningMask
        //    lockbits |= lockbits << 4;
        //    panning &= lockbits;
        //    panning |= mRc.synth.readRegister(Gbs::REG_NR51) & ~lockbits;
        //}

        mRc.synth.setOutputEnable(static_cast<Gbs::OutputFlags>(panning));
        mFlags &= ~FLAGS_PANNING;
    }

    // step timer and check for overflow
    if (mTimer.step()) {
        // timer overflowed, advance pattern iterator to the next row
        // this also means that this step is the last one for the current row
        if (mCursor.next()) {
            // end of pattern
            if (mCommand == PatternCommand::none) {
                // load the next pattern if no command was set
                mCommand = PatternCommand::next;
                mCommandParam = 0;
            }

        }
    }

    return false;
}


template <ChType ch>
void MusicRuntime::update() {
    constexpr int chint = static_cast<int>(ch);
    constexpr bool isFrequencyChannel = ch != ChType::ch4;
    constexpr uint8_t panningBits = 0x11 << chint;

    NoteControl &nc = mNc[chint];
    FrequencyControl *fc = isFrequencyChannel ? mFc + chint : nullptr;
    if constexpr (!isFrequencyChannel) {
        (void)fc;
    }

    auto note = nc.step();

    if (nc.isPlaying()) {

        if (note) {

            uint8_t noteVal = note.value();

            if constexpr (isFrequencyChannel) {
                fc->setNote(noteVal);
                fc->apply();
            } else {
                // CH4
                if (noteVal <= NOTE_NOISE_LAST) {
                    uint8_t noise = NOTE_NOISE_TABLE[noteVal];
                    if (mTimbre[chint]) {
                        // nonzero timbre, 7-bit step width
                        noise |= 0x08;
                    }
                    mRc.synth.writeRegister(Gbs::REG_NR43, noise);
                }

            }

            if (!!(mFlags & (FLAGS_AREN1 << chint))) {
                mRc.synth.restart(ch);
            }

            mPanningMask |= panningBits;
            mFlags |= FLAGS_PANNING;

        }


        if constexpr (isFrequencyChannel) {
            fc->step();
            // write frequency
            mRc.synth.setFrequency(ch, fc->frequency());
        }
    } else if (mPanningMask & panningBits) {
        // no longer playing the note, reset bits in the panning mask
        mPanningMask &= ~panningBits;
        mFlags |= FLAGS_PANNING;
    }
}

bool MusicRuntime::processPatternEffect(Effect effect) {
    
    switch (effect.type) {
        case EffectType::patternGoto:
            if (effect.param <= mLastOrder) {
                mCommand = PatternCommand::jump;
                mCommandParam = effect.param;
            }
            break;
        case EffectType::patternHalt:
            return true;
        case EffectType::patternSkip:
            if (effect.param <= mRowsPerTrack) {
                mCommand = PatternCommand::next;
                mCommandParam = effect.param;
            }
            break;
        case EffectType::setTempo:
            mTimer.setPeriod(effect.param);
            break;
        case EffectType::sfx:
            break; // TODO
    }

    return false;
}

template <ChType ch>
void MusicRuntime::processTrackEffect(Effect effect) {
    constexpr int chint = static_cast<int>(ch);

    switch (effect.type) {
        case EffectType::setEnvelope:
            //
            mEnvelope[chint] = effect.param;
            if constexpr (ch != ChType::ch3) {
                if ((effect.param & 0x7) == 0) {
                    // constant volume, disable AREN
                    mFlags &= ~(FLAGS_AREN1 << chint);
                } else {
                    // envelope is increasing/decreasing, enable AREN
                    mFlags |= FLAGS_AREN1 << chint;
                }
            }
            writeEnvelope<ch>(effect.param);
            break;
        case EffectType::setTimbre:
            mTimbre[chint] = effect.param & 0x3;
            writeTimbre<ch>(mTimbre[chint]);
            break;
        case EffectType::setPanning:
            mPanning &= static_cast<uint8_t>(~(0x11 << chint));
            mPanning |= (effect.param & 0x11) << chint;
            mFlags |= FLAGS_PANNING;
            break;
        case EffectType::setSweep:
            // this effect only modifies CH1's sweep register and can be used on any channel
            mRc.synth.writeRegister(Gbs::REG_NR10, effect.param);
            mRc.synth.restart(ChType::ch1);
            break;
        case EffectType::delayedCut:
            mNc[chint].noteCut(effect.param);
            break;
        case EffectType::delayedNote:
            mNoteDelay = effect.param;
            break;
        case EffectType::lock:
            break; // TODO
        default:
            break; // should never happen
    }
}

bool MusicRuntime::processFreqEffect(Effect effect, FrequencyControl &fc) {
    switch (effect.type) {
        case EffectType::arpeggio:
            fc.setArpeggio(effect.param);
            return true;
        case EffectType::pitchUp:
            fc.setPitchSlide(FrequencyControl::SlideDirection::down, effect.param);
            return true;
        case EffectType::pitchDown:
            fc.setPitchSlide(FrequencyControl::SlideDirection::up, effect.param);
            return true;
        case EffectType::autoPortamento:
            fc.setPortamento(effect.param);
            return false;
        case EffectType::vibrato:
            fc.setVibrato(effect.param);
            return false;
        case EffectType::vibratoDelay:
            fc.setVibratoDelay(effect.param);
            return false;
        case EffectType::tuning:
            fc.setTune(effect.param);
            return false;
        case EffectType::noteSlideUp:
            fc.setNoteSlide(FrequencyControl::SlideDirection::up, effect.param);
            return true;
        case EffectType::noteSlideDown:
            fc.setNoteSlide(FrequencyControl::SlideDirection::down, effect.param);
            return true;
        default:
            return false;
    }
}

template <ChType ch>
void MusicRuntime::writeEnvelope(uint8_t envelope) {

    if constexpr (ch == ChType::ch3) {

        Waveform *waveform = mRc.waveTable[envelope];
        if (waveform == nullptr) {
            return; // do nothing if no waveform exists
        }
        // on real hardware we would have to disable the generator by
        // writing 0x00 to NR30 to prevent corruption. The emulator does
        // not corrupt wave ram on retrigger so we can skip this step.

        // copy waveform to wave ram
        mRc.synth.hardware().gen3.copyWave(*waveform);

    } else {
        static constexpr uint16_t NRx2[] = { Gbs::REG_NR12, Gbs::REG_NR22, 0, Gbs::REG_NR42 };
        // write envelope
        // [rNRx2] <- envelope
        mRc.synth.writeRegister(NRx2[static_cast<int>(ch)], envelope);
    }
    // retrigger
    mRc.synth.restart(ch);
}

template <ChType ch>
void MusicRuntime::writeTimbre(uint8_t timbre) {
    auto &hf = mRc.synth.hardware();

    if constexpr (ch == ChType::ch1 || ch == ChType::ch2) {
        PulseGen *gen;
        if constexpr (ch == ChType::ch1) {
            gen = &hf.gen1;
        } else {
            gen = &hf.gen2;
        }

        gen->setDuty(static_cast<Gbs::Duty>(timbre));
    } else if constexpr (ch == ChType::ch3) {
        if (timbre == 1) {
            timbre = 3;
        } else if (timbre == 3) {
            timbre = 1;
        }
        hf.gen3.setVolume(static_cast<Gbs::WaveVolume>(timbre));

    } else {
        uint8_t nr43 = hf.gen4.readRegister();
        if (timbre) {
            nr43 |= 0x08;
        } else {
            nr43 &= ~0x08;
        }
        hf.gen4.writeRegister(nr43);
    }
}


}
