

#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"
#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/synth/Synth.hpp"

#include "audio.hpp"

using audio::PlaybackQueue;

using trackerboy::Synth;
using trackerboy::Instrument;
using trackerboy::InstrumentTable;
using trackerboy::Instruction;
using trackerboy::Pattern;
using trackerboy::PatternRuntime;
using trackerboy::ChType;
using trackerboy::WaveTable;

static constexpr float SAMPLING_RATE = 44100;

static void outputFrame(int16_t *framePtr, size_t framesize, PlaybackQueue &queue) {
    int16_t *fp = framePtr;
    size_t toWrite = framesize;
    size_t nwritten = 0;
    for (;;) {
        nwritten = queue.write(fp, toWrite);
        if (nwritten == toWrite) {
            break;
        }
        Pa_Sleep(10);
        toWrite -= nwritten;
        fp += nwritten * 2;
    }
}


int main() {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    Synth synth(SAMPLING_RATE);
    auto &mixer = synth.getMixer();
    mixer.setTerminalEnable(trackerboy::Gbs::TERM_BOTH, true);
    mixer.setTerminalVolume(trackerboy::Gbs::TERM_BOTH, trackerboy::Gbs::MAX_TERM_VOLUME);
    PlaybackQueue pb(SAMPLING_RATE);

    Instrument inst;
    auto &prgm = inst.getProgram();
    prgm.push_back({ 1, 0x8E, 0x0, 0xF0, trackerboy::NOTE_NONE });
    //prgm.push_back({ 1, 0x87, 0x0, 0x00, trackerboy::NOTE_NONE });

    InstrumentTable itable;
    WaveTable wtable;
    itable.add(inst);



    Pattern pat(69);
    pat.setNote(ChType::ch1, 16, trackerboy::NOTE_C + trackerboy::OCTAVE_6);
    pat.setInstrument(ChType::ch1, 16, 0);

    pat.setNote(ChType::ch1, 18, trackerboy::NOTE_CUT);
    
    pat.setNote(ChType::ch1, 22, trackerboy::NOTE_B + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 22, 0);

    pat.setNote(ChType::ch1, 24, trackerboy::NOTE_Bb + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 24, 0);

    pat.setNote(ChType::ch1, 26, trackerboy::NOTE_CUT);

    pat.setNote(ChType::ch1, 30, trackerboy::NOTE_A + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 30, 0);

    pat.setNote(ChType::ch1, 32, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 32, 0);

    pat.setNote(ChType::ch1, 34, trackerboy::NOTE_CUT);

    pat.setNote(ChType::ch1, 37, trackerboy::NOTE_Eb + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 37, 0);

    pat.setNote(ChType::ch1, 44, trackerboy::NOTE_CUT);

    pat.setNote(ChType::ch1, 0x2F, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 0x2F, 0);

    pat.setNote(ChType::ch1, 0x31, trackerboy::NOTE_G + trackerboy::OCTAVE_5);
    pat.setInstrument(ChType::ch1, 0x31, 0);

    pat.setNote(ChType::ch1, 0x35, trackerboy::NOTE_CUT);

    pat.setNote(ChType::ch1, 0x41, trackerboy::NOTE_Gb + trackerboy::OCTAVE_6);
    pat.setInstrument(ChType::ch1, 0x41, 0);

    pat.setNote(ChType::ch1, 0x42, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
    pat.setInstrument(ChType::ch1, 0x42, 0);

    



    PatternRuntime runtime(ChType::ch1);
    runtime.setPattern(&pat);


    constexpr size_t tempo = 150;
    constexpr size_t rowsPerBeat = 8;
    constexpr size_t framesPerRow = 3600 / (tempo * rowsPerBeat);
    constexpr size_t samplesPerFrame = SAMPLING_RATE / 60;

    std::vector<int16_t> frameBuf;
    frameBuf.resize(samplesPerFrame * 2);
    int16_t *framePtr = frameBuf.data();

    pb.start();

    do {
        for (size_t frames = 0; frames != framesPerRow; ++frames) {
            runtime.step(synth, itable, wtable);
            synth.fill(framePtr, samplesPerFrame);
            outputFrame(framePtr, samplesPerFrame, pb);
        }
        
    } while (!runtime.nextRow());

    pb.stop(true);

    Pa_Terminate();
    return 0;

}