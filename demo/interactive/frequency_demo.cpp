
#include "interactive.hpp"

#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/note.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cctype>

class FrequencyDemo : public InteractiveDemo {

public:
    FrequencyDemo() :
        InteractiveDemo(),
        mFc(&mToneFc),
        mToneFc(),
        mNoiseFc()
    {
    }


protected:

    int init() override {
        
        auto &apu = mSynth.apu();
        apu.writeRegister(gbapu::Apu::REG_NR52, 0x80);
        apu.writeRegister(gbapu::Apu::REG_NR50, 0x77);
        apu.writeRegister(gbapu::Apu::REG_NR51, 0x11);
        apu.writeRegister(gbapu::Apu::REG_NR12, 0x80);
        apu.writeRegister(gbapu::Apu::REG_NR14, 0x80);
        apu.writeRegister(gbapu::Apu::REG_NR42, 0x80);
        apu.writeRegister(gbapu::Apu::REG_NR44, 0x80);

        std::cout << "Frequency effects demo. Syntax: <note> [effect]..." << std::endl;
        std::cout << "Available effects: 0xy 1xx 2xx 3xx 4xy 5xx Pxx Qxy Rxy" << std::endl << std::endl;

        std::cout << "Tone mode" << std::endl;

        return 0;
    }

    bool processLine(std::string const& line) override {

        if (line.size() == 1) {

            switch (tolower(line[0])) {
                case 'q':
                    return true;
                case 't': {
                    // tone mode
                    lock();
                    auto &apu = mSynth.apu();
                    apu.writeRegister(gbapu::Apu::REG_NR51, 0x11);
                    mFc = &mToneFc;
                    unlock();
                    std::cout << "Tone mode" << std::endl;
                    return false;
                }
                case 'n': {
                    // noise mode
                    lock();
                    auto &apu = mSynth.apu();
                    apu.writeRegister(gbapu::Apu::REG_NR51, 0x88);
                    mFc = &mNoiseFc;
                    unlock();
                    std::cout << "Noise mode" << std::endl;
                    return false;
                }
            }
        }


        std::istringstream iss(line);
        std::istream_iterator<std::string> iter(iss), end;

        trackerboy::FrequencyControl::Parameters params;

        if (iter != end) {
            if (*iter != ".") {
                unsigned long noteIn;
                try {
                    noteIn = std::stoul(line, nullptr, 10);
                } catch (std::invalid_argument e) {
                    std::cout << "Invalid number" << std::endl;
                    return false;
                } catch (std::out_of_range e) {
                    std::cout << "number too large" << std::endl;
                    return false;
                }

                if (noteIn > trackerboy::NOTE_LAST) {
                    std::cout << "invalid note" << std::endl;
                    return false;
                }

                params.setNote((uint8_t)noteIn);
            }

            ++iter;
        }

        // effects
        while (iter != end) {
            auto effect = *iter;
            if (effect.size() != 3) {
                std::cout << "invalid effect" << std::endl;
                return false;
            }

            char effectChar = toupper(effect[0]);
            uint8_t param;
            try {
                param = std::stoul(effect.substr(1, 2), nullptr, 16);
            } catch (std::invalid_argument e) {
                std::cout << "Invalid parameter" << std::endl;
                return false;
            } catch (std::out_of_range e) {
                std::cout << "number too large" << std::endl;
                return false;
            }

            trackerboy::EffectType type;
            switch (effectChar) {
                case '0':
                    type = trackerboy::EffectType::arpeggio;
                    break;
                case '1':
                    type = trackerboy::EffectType::pitchUp;
                    break;
                case '2':
                    type = trackerboy::EffectType::pitchDown;
                    break;
                case '3':
                    type = trackerboy::EffectType::autoPortamento;
                    break;
                case '4':
                    type = trackerboy::EffectType::vibrato;
                    break;
                case '5':
                    type = trackerboy::EffectType::vibratoDelay;
                    break;
                case 'P':
                    type = trackerboy::EffectType::tuning;
                    break;
                case 'Q':
                    type = trackerboy::EffectType::noteSlideUp;
                    break;
                case 'R':
                    type = trackerboy::EffectType::noteSlideDown;
                    break;
                default:
                    std::cout << "unknown effect" << std::endl;
                    return false;
            }
            params.setEffect(type, param);

            ++iter;
        }

        lock();
        mFc->apply(params);
        unlock();

        return false;
    }

    void runFrame() override {
        
        auto &apu = mSynth.apu();
        mFc->step();
        auto freq = mFc->frequency();
        if (mFc == &mToneFc) {
            apu.writeRegister(gbapu::Apu::REG_NR13, freq & 0xFF);
            apu.writeRegister(gbapu::Apu::REG_NR14, freq >> 8);
        } else {
            apu.writeRegister(gbapu::Apu::REG_NR43, trackerboy::NOTE_NOISE_TABLE[freq / trackerboy::NoiseFrequencyControl::UNITS_PER_NOTE]);
        }

    }


private:
    trackerboy::FrequencyControl *mFc;
    trackerboy::ToneFrequencyControl mToneFc;
    trackerboy::NoiseFrequencyControl mNoiseFc;

};


int main() {
    FrequencyDemo demo;
    return demo.exec();
}
