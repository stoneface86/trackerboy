

#include "trackerboy/export/Wav.hpp"
#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"
#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/synth/Synth.hpp"

#include "audio.hpp"

#include <fstream>

using audio::PlaybackQueue;

using trackerboy::Synth;
using trackerboy::Instrument;
using trackerboy::InstrumentTable;
using trackerboy::Instruction;
using trackerboy::Pattern;
using trackerboy::PatternRuntime;
using trackerboy::Track;
using trackerboy::TrackRuntime;
using trackerboy::ChType;
using trackerboy::WaveTable;
using trackerboy::Wav;

static constexpr float SAMPLING_RATE = 44100;

static void outputFrame(float *framePtr, size_t framesize, PlaybackQueue &queue) {
    float *fp = framePtr;
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
    PlaybackQueue pb(SAMPLING_RATE);

    InstrumentTable itable;
    WaveTable wtable;

    Instrument &inst = itable.insert(0, "test instrument");
    auto &prgm = inst.getProgram();
    prgm.push_back({ 1, 0x8E, 0x0, 0xF1, trackerboy::NOTE_NONE });


    Pattern pat(128);
    Track track = pat.track(ChType::ch1);

    track.setNote(16, trackerboy::NOTE_C + trackerboy::OCTAVE_6);
    track.setInstrument(16, 0);

    track.setNote(18, trackerboy::NOTE_CUT);
    
    track.setNote(22, trackerboy::NOTE_B + trackerboy::OCTAVE_5);
    track.setInstrument(22, 0);

    track.setNote(24, trackerboy::NOTE_Bb + trackerboy::OCTAVE_5);
    track.setInstrument(24, 0);

    track.setNote(26, trackerboy::NOTE_CUT);

    track.setNote(30, trackerboy::NOTE_A + trackerboy::OCTAVE_5);
    track.setInstrument(30, 0);

    track.setNote(32, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    track.setInstrument(32, 0);

    track.setNote(34, trackerboy::NOTE_CUT);

    track.setNote(37, trackerboy::NOTE_Eb + trackerboy::OCTAVE_5);
    track.setInstrument(37, 0);

    track.setNote(44, trackerboy::NOTE_CUT);

    track.setNote(0x2F, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    track.setInstrument(0x2F, 0);

    track.setNote(0x31, trackerboy::NOTE_G + trackerboy::OCTAVE_5);
    track.setInstrument(0x31, 0);

    track.setNote(0x35, trackerboy::NOTE_CUT);

    track.setNote(0x41, trackerboy::NOTE_Gb + trackerboy::OCTAVE_6);
    track.setInstrument(0x41, 0);

    track.setNote(0x42, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
    track.setInstrument(0x42, 0);
    
    constexpr size_t tempo = 150;
    constexpr size_t rowsPerBeat = 8;
    constexpr size_t speed = 3600 / (tempo * rowsPerBeat);
    constexpr size_t samplesPerFrame = SAMPLING_RATE / 60;


    PatternRuntime pr(0x14);
    pr.setPattern(&pat);

    

    std::vector<float> frameBuf;
    frameBuf.resize(samplesPerFrame * 2);
    float *framePtr = frameBuf.data();

    std::ofstream file("pattern_demo.wav", std::ios::binary | std::ios::out);
    Wav wav(file, 2, SAMPLING_RATE);
    wav.begin();
    pb.start();

    bool patternEnded;
    do {
        patternEnded = pr.step(synth, itable, wtable);
        synth.fill(framePtr, samplesPerFrame);
        outputFrame(framePtr, samplesPerFrame, pb);
        wav.write(framePtr, samplesPerFrame);
    } while (!patternEnded);

    pb.stop(true);

    wav.finish();
    file.close();

    Pa_Terminate();
    return 0;

}
