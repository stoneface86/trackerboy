

#include "trackerboy/export/Wav.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/note.hpp"
#include "trackerboy/data/Module.hpp"

#include "audio.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>

using audio::PlaybackQueue;

using namespace trackerboy;

static constexpr float SAMPLING_RATE = 48000;

//#define READ_FILE

void printFrame(Frame &frame) {

    std::cout << frame.time << ": " << static_cast<int>(frame.row) << " / " << static_cast<int>(frame.order) << std::endl;
}



int main() {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    Synth synth(SAMPLING_RATE);
    PlaybackQueue pb(audio::SR_48000);

    Module mod;
    #ifdef READ_FILE
    {
        std::ifstream modfile("song_demo.tbm", std::ios::binary | std::ios::in);
        mod.deserialize(modfile);
    }
    #endif

    InstrumentTable &itable = mod.instrumentTable();
    WaveTable &wtable = mod.waveTable();
    RuntimeContext rc(synth, itable, wtable);

    #ifndef READ_FILE
    

    {
        auto &inst = itable.insert();
        auto &idata = inst.data();
        idata.envelope = 0x57;
        idata.timbre = 1;
    }

    {
        auto &inst = itable.insert();
        auto &idata = inst.data();
        idata.envelope = 0x77;
        idata.timbre = 0x0;
    }

    auto &triangle = wtable.insert();
    triangle.fromString("0123456789ABCDEFFEDCBA9876543210");

    
    
    auto &songs = mod.songs();
    songs.emplace_back();
    Song &testsong = songs[0];
    testsong.setMode(Song::Mode::speed);
    testsong.setSpeed(0x11);
    testsong.apply();

    auto &orders = testsong.orders();
    orders.clear();
    orders.push_back({ 0, 0, 0, 0 });
    orders.push_back({ 0, 1, 0, 1 });
    orders.push_back({ 0, 0, 0, 0 });
    orders.push_back({ 0, 2, 0, 2 });


    auto &pm = testsong.patterns();
    
    {
        auto &tr = pm.getTrack(ChType::ch4, 0);
        tr.setEffect(0, 0, EffectType::setEnvelope, 0xB1);
        tr.setNote(0x00, NOTE_C + OCTAVE_6);

        tr.setNote(0x04, NOTE_C + OCTAVE_6);

        tr.setNote(0x08, NOTE_C + OCTAVE_6);

        tr.setNote(0x0C, NOTE_G + OCTAVE_6);

        tr.setNote(0x14, NOTE_G + OCTAVE_6);

        tr.setNote(0x1C, NOTE_G + OCTAVE_6);

        tr.setNote(0x24, NOTE_G + OCTAVE_6);

        tr.setNote(0x2C, NOTE_G + OCTAVE_6);

        tr.setNote(0x34, NOTE_G + OCTAVE_6);

        tr.setNote(0x38, NOTE_F + OCTAVE_6);

        tr.setNote(0x3C, NOTE_G + OCTAVE_6);

    }

    {
        auto &tr = pm.getTrack(ChType::ch4, 1);
        tr.setNote(0x00, NOTE_C + OCTAVE_6);

        tr.setNote(0x04, NOTE_C + OCTAVE_6);

        tr.setNote(0x08, NOTE_C + OCTAVE_6);

        tr.setNote(0x0C, NOTE_G + OCTAVE_6);

        tr.setNote(0x14, NOTE_G + OCTAVE_6);

        tr.setNote(0x1C, NOTE_G + OCTAVE_6);

        tr.setNote(0x24, NOTE_G + OCTAVE_6);

        tr.setNote(0x2C, NOTE_G + OCTAVE_6);

        tr.setNote(0x34, NOTE_G + OCTAVE_6);

        tr.setNote(0x38, NOTE_C + OCTAVE_6);

        tr.setNote(0x3C, NOTE_F + OCTAVE_6);

    }

    {
        auto &tr = pm.getTrack(ChType::ch4, 2);
        tr.setNote(0x00, NOTE_C + OCTAVE_6);

        tr.setNote(0x04, NOTE_C + OCTAVE_6);

        tr.setNote(0x08, NOTE_C + OCTAVE_6);

        tr.setNote(0x0C, NOTE_G + OCTAVE_6);

        tr.setNote(0x14, NOTE_G + OCTAVE_6);

        tr.setNote(0x1C, NOTE_G + OCTAVE_6);

        tr.setNote(0x24, NOTE_G + OCTAVE_6);

        tr.setNote(0x2C, NOTE_G + OCTAVE_6);

        tr.setNote(0x34, NOTE_F + OCTAVE_6);

        tr.setNote(0x38, NOTE_F + OCTAVE_6);
        tr.setNote(0x3A, NOTE_F + OCTAVE_6);
        tr.setNote(0x3C, NOTE_F + OCTAVE_6);
        tr.setNote(0x3E, NOTE_F + OCTAVE_6);

    }



    {
        auto &tr = pm.getTrack(ChType::ch3, 0);
        tr.setEffect(0, 0, EffectType::setEnvelope, 0);
        tr.setNote(0x00, NOTE_G + OCTAVE_3);

        tr.setNote(0x0C, NOTE_CUT);

    }

    {
        auto &tr = pm.getTrack(ChType::ch2, 0);
        //tr.setEffect(0, 0, EffectType::setTimbre, 1);
        //tr.setEffect(0, 1, EffectType::setEnvelope, 0x57);
        tr.setInstrument(0, 0);
        tr.setNote(0x00, NOTE_G + OCTAVE_5);

        tr.setNote(0x08, NOTE_CUT);

        /*tr.setEffect(0x0C, 0, EffectType::setTimbre, 0);
        tr.setEffect(0x0C, 1, EffectType::setEnvelope, 0x77);*/
        tr.setInstrument(0x0C, 1);
        tr.setNote(0x0C, NOTE_F + OCTAVE_3);

        tr.setNote(0x10, NOTE_G + OCTAVE_3);

        tr.setNote(0x16, NOTE_CUT);

        tr.setNote(0x18, NOTE_Bb + OCTAVE_3);

        tr.setNote(0x1C, NOTE_C + OCTAVE_4);

        tr.setNote(0x22, NOTE_CUT);

        tr.setNote(0x24, NOTE_F + OCTAVE_3);

        tr.setNote(0x28, NOTE_G + OCTAVE_3);

        tr.setNote(0x2E, NOTE_CUT);

        tr.setNote(0x30, NOTE_Bb + OCTAVE_2);

        tr.setNote(0x34, NOTE_C + OCTAVE_3);

        tr.setNote(0x38, NOTE_F + OCTAVE_2);

        tr.setNote(0x3C, NOTE_G + OCTAVE_2);
    }

    {
        auto &tr = pm.getTrack(ChType::ch2, 1);
        tr.setEffect(0, 0, EffectType::setTimbre, 1);
        tr.setEffect(0, 1, EffectType::setEnvelope, 0x57);
        tr.setNote(0x00, NOTE_G + OCTAVE_5);


        tr.setEffect(0x0C, 0, EffectType::setTimbre, 0);
        tr.setEffect(0x0C, 1, EffectType::setEnvelope, 0x77);
        tr.setNote(0x0C, NOTE_F + OCTAVE_3);

        tr.setNote(0x10, NOTE_G + OCTAVE_3);

        tr.setNote(0x16, NOTE_CUT);

        tr.setNote(0x18, NOTE_Bb + OCTAVE_3);

        tr.setNote(0x1C, NOTE_C + OCTAVE_4);

        tr.setNote(0x22, NOTE_CUT);

        tr.setNote(0x24, NOTE_F + OCTAVE_3);

        tr.setNote(0x28, NOTE_G + OCTAVE_3);

        tr.setNote(0x2E, NOTE_CUT);

        tr.setNote(0x30, NOTE_C + OCTAVE_3);

        tr.setNote(0x34, NOTE_Bb + OCTAVE_2);

        tr.setNote(0x38, NOTE_G + OCTAVE_2);

        tr.setNote(0x3C, NOTE_F + OCTAVE_2);
    }

    {
        auto &tr = pm.getTrack(ChType::ch2, 2);
        tr.setEffect(0, 0, EffectType::setTimbre, 1);
        tr.setEffect(0, 1, EffectType::setEnvelope, 0x57);
        tr.setNote(0x00, NOTE_G + OCTAVE_5);

        tr.setNote(0x08, NOTE_CUT);

        tr.setEffect(0x0C, 0, EffectType::setTimbre, 0);
        tr.setEffect(0x0C, 1, EffectType::setEnvelope, 0x77);
        tr.setNote(0x0C, NOTE_F + OCTAVE_3);

        tr.setNote(0x10, NOTE_G + OCTAVE_3);

        tr.setNote(0x16, NOTE_CUT);

        tr.setNote(0x18, NOTE_C + OCTAVE_4);
        tr.setNote(0x1A, NOTE_D + OCTAVE_4);
        tr.setNote(0x1C, NOTE_C + OCTAVE_4);
        tr.setNote(0x1E, NOTE_Bb + OCTAVE_3);
        tr.setNote(0x20, NOTE_G + OCTAVE_3);

        tr.setNote(0x24, NOTE_C + OCTAVE_4);
        tr.setNote(0x26, NOTE_D + OCTAVE_4);
        tr.setNote(0x28, NOTE_C + OCTAVE_4);
        tr.setNote(0x2A, NOTE_Bb + OCTAVE_3);
        tr.setNote(0x2C, NOTE_G + OCTAVE_3);

        tr.setNote(0x30, NOTE_C + OCTAVE_4);
        tr.setNote(0x32, NOTE_D + OCTAVE_4);
        tr.setNote(0x34, NOTE_C + OCTAVE_4);
        tr.setNote(0x36, NOTE_Bb + OCTAVE_3);
        tr.setNote(0x38, NOTE_G + OCTAVE_3);
        tr.setNote(0x3A, NOTE_F + OCTAVE_3);
        tr.setNote(0x3C, NOTE_C + OCTAVE_3);
        tr.setNote(0x3E, NOTE_Bb + OCTAVE_2);

    }


    {
        auto &tr = pm.getTrack(ChType::ch1, 0);

        tr.setEffect(0, 0, EffectType::setTimbre, 1);
        tr.setEffect(0, 1, EffectType::setEnvelope, 0xA7);
        tr.setNote(0x00, NOTE_G + OCTAVE_3);

        tr.setNote(0x07, NOTE_CUT);

        tr.setNote(0x08, NOTE_G + OCTAVE_3);
        tr.setNote(0x0B, NOTE_CUT);

        tr.setNote(0x0C, NOTE_D + OCTAVE_4);
        tr.setNote(0x0F, NOTE_CUT);

        tr.setNote(0x10, NOTE_D + OCTAVE_4);
        tr.setNote(0x13, NOTE_CUT);

        tr.setNote(0x14, NOTE_Db + OCTAVE_4);
        tr.setNote(0x17, NOTE_CUT);

        tr.setNote(0x18, NOTE_Db + OCTAVE_4);
        tr.setNote(0x1B, NOTE_CUT);

        tr.setNote(0x1C, NOTE_D + OCTAVE_4);
        tr.setNote(0x1F, NOTE_CUT);

        tr.setNote(0x20, NOTE_D + OCTAVE_4);
        tr.setNote(0x23, NOTE_CUT);

        tr.setNote(0x24, NOTE_G + OCTAVE_4);
        tr.setNote(0x27, NOTE_CUT);

        tr.setNote(0x28, NOTE_G + OCTAVE_4);
        tr.setNote(0x2B, NOTE_CUT);

        tr.setNote(0x2C, NOTE_D + OCTAVE_4);

        tr.setNote(0x30, NOTE_A + OCTAVE_4);

        tr.setNote(0x31, NOTE_Bb + OCTAVE_4);

        tr.setNote(0x37, NOTE_CUT);

        tr.setNote(0x38, NOTE_A + OCTAVE_4);

        tr.setNote(0x3F, NOTE_CUT);
    }

    {
        //File file;
        std::ofstream modfile("song_demo.tbm", std::ios::binary | std::ios::out);
        mod.serialize(modfile);
        //file.saveHeader(modfile);
        //file.saveModule(modfile, mod);
        modfile.close();
    }
    #endif


    Engine engine(rc);
    engine.play(mod.songs()[0], 0, 0);
    std::ofstream file("song_demo.wav", std::ios::binary | std::ios::out);
    Wav wav(file, 2, SAMPLING_RATE);
    wav.begin();

    pb.open();

    for (int i = 600; i != 0; --i) {
        Frame frame;
        engine.step(frame);
        printFrame(frame);
        size_t framesize = synth.run();
        //outputFrame(synth.buffer(), framesize, pb);
        pb.writeAll(synth.buffer(), framesize);
        wav.write(synth.buffer(), framesize);
    }

    pb.stop(true);

    pb.close();

    wav.finish();
    file.close();

    Pa_Terminate();
    return 0;

}
