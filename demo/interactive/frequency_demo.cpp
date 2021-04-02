
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
        InteractiveDemo()
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

        std::cout << "Frequency effects demo. Syntax: <note> [effect]..." << std::endl;
        std::cout << "Available effects: 0xy 1xx 2xx 3xx 4xy 5xx Pxx Qxy Rxy" << std::endl << std::endl;


        return 0;
    }

    bool processLine(std::string const& line) override {


        if (line[0] == 'q') {
            return true;
        }


        std::istringstream iss(line);
        std::istream_iterator<std::string> iter(iss), end;

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

                mFc.setNote((uint8_t)noteIn);
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

            switch (effectChar) {
                case '0':
                    mFc.setArpeggio(param);
                    break;
                case '1':
                    mFc.setPitchSlide(trackerboy::FrequencyControl::SlideDirection::up, param);
                    break;
                case '2':
                    mFc.setPitchSlide(trackerboy::FrequencyControl::SlideDirection::down, param);
                    break;
                case '3':
                    mFc.setPortamento(param);
                    break;
                case '4':
                    mFc.setVibrato(param);
                    break;
                case '5':
                    mFc.setVibratoDelay(param);
                    break;
                case 'P':
                    mFc.setTune(param);
                    break;
                case 'Q':
                    mFc.setNoteSlide(trackerboy::FrequencyControl::SlideDirection::up, param);
                    break;
                case 'R':
                    mFc.setNoteSlide(trackerboy::FrequencyControl::SlideDirection::down, param);
                    break;
                default:
                    std::cout << "unknown effect" << std::endl;
                    break;
            }


            ++iter;
        }

        lock();
        mFc.apply();
        unlock();

        return false;
    }

    void runFrame() override {
        
        auto &apu = mSynth.apu();
        mFc.step();
        auto freq = mFc.frequency();
        apu.writeRegister(gbapu::Apu::REG_NR13, freq & 0xFF);
        apu.writeRegister(gbapu::Apu::REG_NR14, freq >> 8);

    }


private:
    trackerboy::FrequencyControl mFc;

};


int main() {
    FrequencyDemo demo;
    return demo.exec();
}
