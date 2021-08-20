
#include "interactive.hpp"
#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <optional>

using namespace trackerboy;
using namespace std::string_literals;

template <typename T>
std::optional<T> parseInteger(std::string const& str, int base = 10) {
    unsigned long num;
    try {
        num = std::stoul(str, nullptr, base);
    } catch (std::exception e) {
        return std::nullopt;
    }

    return static_cast<T>(num);
}


class ApuDemo : public InteractiveDemo {

public:

    ApuDemo() :
        InteractiveDemo()
    {
    }

protected:

    virtual int init() override {
        std::cout << "APU demo. Set a register by typing <reg> <value>, enter q to quit" << std::endl;
        std::cout << "Example: 11 $80 -> sets CH1 duty to 50% (nr11 = 0x80)" << std::endl;
        return 0;
    }

    virtual bool processLine(std::string const& line) override {
        if (line[0] == 'q') {
            return true;
        } else {

            std::istringstream iss(line);
            std::istream_iterator<std::string> iter(iss), end;

            // get the register
            uint8_t reg;
            auto num = parseInteger<unsigned>(*iter);
            if (!num) {
                std::cout << "Invalid register" << std::endl;
                return false;
            }

            int channel = *num / 10;
            int regNo = *num % 10;
            if (channel < 1 || channel > 5) {
                std::cout << "invalid register" << std::endl;
                return false;
            }

            if (channel == 5) {
                if (regNo > 2) {
                    std::cout << "invalid register" << std::endl;
                    return false;
                }

                reg = gbapu::Apu::REG_NR50 + regNo;
            } else {
                if (regNo > 4) {
                    std::cout << "invalid register" << std::endl;
                    return false;
                }

                reg = gbapu::Apu::REG_NR10 + ((channel - 1) * 5) + regNo;
            }
            
            iter++;

            // get the value
            if (iter != end) {
                auto valstr = *iter;
                std::optional<uint8_t> value;
                if (valstr[0] == '$') {
                    // hex
                    value = parseInteger<uint8_t>(valstr.substr(1), 16);
                } else {
                    // decimal
                    value = parseInteger<uint8_t>(valstr);
                }

                if (value) {
                    lock();
                    mSynth.apu().writeRegister(reg, *value);
                    unlock();
                } else {
                    std::cout << "invalid number" << std::endl;
                    return false;
                }

            } else {
                // read the register
                lock();
                auto read = mSynth.apu().readRegister(reg);
                unlock();
                std::cout << "$" << std::hex << std::setw(2) << std::setfill('0') << (unsigned)read << std::endl;
            }
            return false;

        }
    }

    void runFrame() override {

    }

};


int main() {
    ApuDemo demo;
    return demo.exec();
}
