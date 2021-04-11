
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"
#include "internal/enumutils.hpp"

namespace trackerboy {


//
// Apply a row operation to this runtime
//
//void InstrumentRuntimeBase::apply(RuntimeContext const& rc, Operation const& op) {
//    bool retriggerInst = false;
//
//    std::optional<uint8_t> envelopeToSet;
//
//    if (op.note) {
//        retriggerInst = true;
//    }
//
//    if (op.instrument) {
//        // change the current instrument
//        auto inst = rc.instList[*op.instrument];
//        if (inst) {
//            mInstrument = std::move(inst);
//            retriggerInst = true;
//        }
//        // for unknown instruments, do nothing
//    }
//
//    if (retriggerInst && mInstrument) {
//        // restart instrument sequences if
//        //  * a note was triggerred
//        //  * the current instrument changed
//        //  * both of the above
//        
//        // set envelope
//        envelopeToSet = mInstrument->queryEnvelope();
//
//        // restart sequences
//        mPanningSequence = mInstrument->sequence(Instrument::SEQUENCE_PANNING).enumerator();
//        mTimbreSequence = mInstrument->sequence(Instrument::SEQUENCE_TIMBRE).enumerator();
//    }
//
//    if (!envelopeToSet) {
//        envelopeToSet = op.envelope;
//    }
//
//    if (op.timbre) {
//        mTimbre = *op.timbre;
//
//    }
//
//    if (op.duration) {
//        mCutCounter = op.duration;
//    }
//
//    //
//    // Envelope priority:
//    //  1. if the instrument was retriggered and the instrument has an envelope
//    //  2. the Exx effect was used
//    //  3. no envelope to set otherwise
//    //
//    if (envelopeToSet) {
//        auto envelope = *envelopeToSet;
//        if (envelope != mCurrentEnvelope) {
//            // set the envelope
//            if (mCh == ChType::ch3) {
//                // set the waveform
//            } else {
//                // set the envelope register
//
//            }
//            mCurrentEnvelope = envelope;
//        }
//    }
//
//    mFc.apply(op);
//
//}

InstrumentRuntime::InstrumentRuntime(FrequencyControl &fc) :
    mFc(fc),
    mInstrument(),
    mRestart(false),
    mPanningSequence(),
    mTimbreSequence()
{
}

void InstrumentRuntime::setInstrument(std::shared_ptr<Instrument> instrument) {
    mInstrument = std::move(instrument);
    mRestart = true;
}

void InstrumentRuntime::restart() {
    mRestart = true;
}

void InstrumentRuntime::step(ChannelState &state) {

    if (mRestart) {
        if (mInstrument) {
            auto envelope = mInstrument->queryEnvelope();
            if (envelope) {
                state.envelope = *envelope;
            }

            mPanningSequence = mInstrument->sequence(Instrument::SEQUENCE_PANNING).enumerator();
            mTimbreSequence = mInstrument->sequence(Instrument::SEQUENCE_TIMBRE).enumerator();
            mFc.useInstrument(*mInstrument);
        }

        mRestart = false;
    }

    auto panning = mPanningSequence.next();
    if (panning) {
        state.panning = *panning;
    }

    auto timbre = mTimbreSequence.next();
    if (timbre) {
        state.timbre = *timbre;
    }

    mFc.step();
    state.frequency = mFc.frequency();

}

//void InstrumentRuntimeBase::step(RuntimeContext const& rc) {
//
//    if (mPlaying) {
//
//        if (mCutCounter) {
//            if (*mCutCounter == 0) {
//                // cut the note
//                mCutCounter.reset();
//                mPlaying = false;
//                return;
//            } else {
//                (*mCutCounter)--;
//            }
//        }
//
//
//        auto panning = mPanningSequence.next();
//        if (panning) {
//            
//            std::optional<uint8_t> setval;
//            /*switch (static_cast<Panning>(*panning)) {
//                case Panning::left:
//                    setval = (uint8_t)0x10;
//                    break;
//                case Panning::right:
//                    setval = (uint8_t)0x01;
//                    break;
//                case Panning::middle:
//                    setval = (uint8_t)0x11;
//                    break;
//                default:
//                    break;
//            }
//            if (setval) {
//                auto nr51 = rc.apu.readRegister(gbapu::Apu::REG_NR51);
//                nr51 &= ~((uint8_t)0x11 << +mCh);
//                nr51 |= *setval << +mCh;
//                rc.apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
//            }*/
//
//        }
//
//        uint8_t currentTimbre;
//        auto timbre = mTimbreSequence.next();
//        if (timbre) {
//            currentTimbre = *timbre;
//        } else {
//            currentTimbre = mTimbre;
//        }
//
//        mFc.step();
//        if (mCh == ChType::ch4) {
//            auto nr43 = NOTE_NOISE_TABLE[mFc.frequency() / NoiseFrequencyControl::UNITS_PER_NOTE];
//            if (currentTimbre) {
//                nr43 |= 0x08;
//            }
//            rc.apu.writeRegister(gbapu::Apu::REG_NR43, nr43);
//        } else {
//            auto freq = mFc.frequency();
//            rc.apu.writeRegister(mRegAddr + 3, freq & 0xFF);
//            rc.apu.writeRegister(mRegAddr + 4, freq >> 8);
//
//        }
//
//
//    }
//
//}


}
