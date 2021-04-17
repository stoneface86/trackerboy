

#include "trackerboy/data/Module.hpp"
#include "trackerboy/InstrumentPreview.hpp"
#include "trackerboy/Synth.hpp"

#include "miniaudio.h"

using namespace trackerboy;

constexpr unsigned SAMPLERATE = 48000;

int main() {

    Module mod;

    auto &ilist = mod.instrumentTable();
    auto &wlist = mod.waveformTable();
    
    // sample instrument with a looping arp sequence
    auto &inst = ilist.insert();
    inst.setEnvelope(0xF4);
    inst.setEnvelopeEnable(true);
    {
        auto &seq = inst.sequence(Instrument::SEQUENCE_ARP);
        auto &seqdata = seq.data();
        seqdata = { 0, 0, 7, 7, 4, 4, 11, 11 };
        seq.setLoop(0);
    }
    {
        auto &seq = inst.sequence(Instrument::SEQUENCE_TIMBRE);
        auto &seqdata = seq.data();
        seqdata = { 0 };
        //seq.setLoop(0);
    }



    ma_encoder_config config = ma_encoder_config_init(ma_resource_format_wav, ma_format_s16, 2, SAMPLERATE);
    ma_encoder encoder;
    auto result = ma_encoder_init_file("inst_demo.wav", &config, &encoder);
    if (result != MA_SUCCESS) {
        return 1;
    }


    Synth synth(SAMPLERATE);
    GbApu apu(synth.apu());
    RuntimeContext rc(apu, ilist, wlist);

    std::vector<int16_t> buffer;
    buffer.resize(synth.framesize() * 2);

    InstrumentPreview preview;
    preview.setInstrument(ilist.getShared(0));
    uint8_t note = 36;
    

    for (int i = 0; i != 200; ++i) {
        if (i % 50 == 0) {
            preview.play(note);
            note += 12;
        }
        
        preview.step(rc);
        synth.run();

        auto samples = synth.apu().readSamples(buffer.data(), buffer.size());
        ma_encoder_write_pcm_frames(&encoder, buffer.data(), samples);
    }

    ma_encoder_uninit(&encoder);


    return 0;
}
