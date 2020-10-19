
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/note.hpp"

namespace trackerboy {

MusicRuntime::MusicRuntime(RuntimeContext rc, ChannelControl &chCtrl, Song &song, uint8_t orderNo, uint8_t patternRow) :
    mRc(rc),
    mSong(song),
    mOrderCounter(orderNo),
    mRowCounter(patternRow),
    mLastOrder(static_cast<uint8_t>(song.orders().size() - 1)),
    mRowsPerTrack(song.patterns().rowSize()),
    mCommand(PatternCommand::none),
    mCommandParam(0),
    mEnvelope{ 0xF0, 0xF0, 0x00, 0xF0 },
    mTimbre{ 0, 0, 3, 0 },
    mPanning(0xFF),
    mPanningMask(0x00),
    mNoteDelay(0),
    mFlags(FLAGS_DEFAULT),
    mChCtrl(chCtrl)
{
    mCursor.setPattern(song.getPattern(orderNo), patternRow);
    mTimer.setPeriod(song.speed());
}

Speed MusicRuntime::speed() const noexcept {
    return mTimer.period();
}

uint8_t MusicRuntime::currentOrder() const noexcept {
    return mOrderCounter;
}

uint8_t MusicRuntime::currentRow() const noexcept {
    return mRowCounter;
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

    if (!!(row.flags & TrackRow::COLUMN_INST)) {
        auto instrument = mRc.instTable[row.instrumentId];
        if (instrument != nullptr) {
            auto &idata = instrument->data();
            setTimbre<ch>(idata.timbre);
            setEnvelope<ch>(idata.envelope);
            if (idata.panning) {
                setPanning<ch>(idata.panning);
            }

            auto &nc = mNc[chint];
            mNoteDelay = idata.delay;
            if (idata.duration) {
                nc.noteCut(idata.duration);
            }

            if constexpr (ch != ChType::ch4) {
                auto &fc = mFc[chint];
                fc.setTune(static_cast<uint8_t>(idata.tune) + 0x80);
                fc.setVibrato(idata.vibrato);
                fc.setVibratoDelay(idata.vibratoDelay);
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

void MusicRuntime::reload(ChType ch) {

    switch (ch) {
        case ChType::ch1:
            reloadImpl<ChType::ch1>();
            break;
        case ChType::ch2:
            reloadImpl<ChType::ch2>();
            break;
        case ChType::ch3:
            reloadImpl<ChType::ch3>();
            break;
        case ChType::ch4:
            reloadImpl<ChType::ch4>();
            break;
    }
}

template <ChType ch>
void MusicRuntime::reloadImpl() {
    constexpr int chint = static_cast<int>(ch);

    ChannelControl::writeEnvelope(ch, mRc, mEnvelope[chint]);
    if constexpr (ch == ChType::ch4) {
        // TODO: keep a copy of the NR43 register to restore here
    } else {
        //writeTimbre<ch>(mTimbre[chint]);
        ChannelControl::writeTimbre(ch, mRc, mTimbre[chint]);
        ChannelControl::writeFrequency(ch, mRc, mFc[chint].frequency());
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
                mRowCounter = mCommandParam;
                mCursor.setPattern(mSong.getPattern(mOrderCounter), mCommandParam);
                mCommand = PatternCommand::none;
                break;
            case PatternCommand::jump:
                mRowCounter = 0;
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

        uint8_t lockbits = mChCtrl.lockbits();
        if (lockbits) {
            // when one or more channels are unlocked:
            // NR51 <- ((y & z) & ~w) | (x & w)
            // where
            //  w: channel lock bits
            //  x: current value of NR51
            //  y: mPanning
            //  z: mPanningMask
            lockbits |= lockbits << 4;
            panning &= ~lockbits;
            panning |= mRc.synth.readRegister(Gbs::REG_NR51) & lockbits;
        }

        //mRc.synth.setOutputEnable(static_cast<Gbs::OutputFlags>(panning));
        mRc.synth.writeRegister(Gbs::REG_NR51, panning);
        mFlags &= ~FLAGS_PANNING;
    }

    // step timer and check for overflow
    if (mTimer.step()) {
        // timer overflowed, advance pattern iterator to the next row
        // this also means that this step is the last one for the current row
        ++mRowCounter;
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

    bool const locked = mChCtrl.isLocked(ch);

    auto note = nc.step();

    if (nc.isPlaying()) {

        uint16_t retrigger = 0;

        if (note) {

            uint8_t noteVal = note.value();

            if (locked && !!(mFlags & (FLAGS_AREN1 << chint))) {
                retrigger = 0x8000;
                //mRc.synth.restart(ch);
            }

            if constexpr (isFrequencyChannel) {
                fc->setNote(noteVal);
                fc->apply();
            } else {
                // CH4
                if (locked && noteVal <= NOTE_NOISE_LAST) {
                    uint8_t noise = NOTE_NOISE_TABLE[noteVal];
                    if (mTimbre[chint]) {
                        // nonzero timbre, 7-bit step width
                        noise |= 0x08;
                    }
                    mRc.synth.writeRegister(Gbs::REG_NR43, noise);
                    mRc.synth.writeRegister(Gbs::REG_NR44, retrigger >> 8);
                }

            }



            mPanningMask |= panningBits;
            mFlags |= FLAGS_PANNING;

        }


        if constexpr (isFrequencyChannel) {
            fc->step();
            // write frequency

            if (locked) {
                ChannelControl::writeFrequency(ch, mRc, retrigger | fc->frequency());
            }
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
        default:
            break; // should never happen
    }

    return false;
}

template <ChType ch>
void MusicRuntime::processTrackEffect(Effect effect) {
    constexpr int chint = static_cast<int>(ch);

    switch (effect.type) {
        case EffectType::setEnvelope:
            //
            setEnvelope<ch>(effect.param);
            break;
        case EffectType::setTimbre:
            setTimbre<ch>(effect.param);
            break;
        case EffectType::setPanning:
            setPanning<ch>(effect.param);
            break;
        case EffectType::setSweep:
            // this effect only modifies CH1's sweep register and can be used on any channel
            mRc.synth.writeRegister(Gbs::REG_NR10, effect.param);
            mRc.synth.writeRegister(Gbs::REG_NR14, 0x80 | (mFc[0].frequency() >> 8));
            break;
        case EffectType::delayedCut:
            mNc[chint].noteCut(effect.param);
            break;
        case EffectType::delayedNote:
            mNoteDelay = effect.param;
            break;
        case EffectType::lock:
            mChCtrl.lock(ch);
            break;
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
void MusicRuntime::setEnvelope(uint8_t envelope) {
    constexpr int chint = static_cast<int>(ch);
    mEnvelope[chint] = envelope;
    if constexpr (ch != ChType::ch3) {
        if ((envelope & 0x7) == 0) {
            // constant volume, disable AREN
            mFlags &= ~(FLAGS_AREN1 << chint);
        } else {
            // envelope is increasing/decreasing, enable AREN
            mFlags |= FLAGS_AREN1 << chint;
        }
    }
    if (mChCtrl.isLocked(ch)) {
        uint8_t freqMsb;
        if constexpr (ch == ChType::ch4) {
            freqMsb = 0;
        } else {
            freqMsb = mFc[chint].frequency() >> 8;
        }
        ChannelControl::writeEnvelope(ch, mRc, envelope, freqMsb);
    }
}

template <ChType ch>
void MusicRuntime::setTimbre(uint8_t timbre) {
    constexpr int chint = static_cast<int>(ch);
    mTimbre[chint] = timbre & 0x3;
    if (mChCtrl.isLocked(ch)) {
        ChannelControl::writeTimbre(ch, mRc, mTimbre[chint]);
    }
}

template <ChType ch>
void MusicRuntime::setPanning(uint8_t panning) {
    constexpr int chint = static_cast<int>(ch);
    mPanning &= static_cast<uint8_t>(~(0x11 << chint));
    mPanning |= (panning & 0x11) << chint;
    mFlags |= FLAGS_PANNING;
}


}
