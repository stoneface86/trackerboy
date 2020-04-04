
#pragma once

#include <fstream>
#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

class Instrument {

public:

    enum Flags {

        // ctrl byte flags

        // Bit 7: xctrl override, if set byte is xctrl, ctrl otherwise
        CTRL_FLAG_ISXCTRL       = 0x80,
        // Bit 6: retrigger
        CTRL_FLAG_RESTART       = 0x40,
        // Bit 5: xctrl enable
        CTRL_FLAG_XCTRL_EN      = 0x20,
        // Bit 4: aux1 byte enable
        CTRL_FLAG_AUX1_EN       = 0x10,
        // Bit 3: aux2 byte enable
        CTRL_FLAG_AUX2_EN       = 0x08,
        // Bit 2: settings enable
        CTRL_FLAG_SETTINGS_EN   = 0x04,
        // Bits 0-1: settings
        CTRL_FLAG_SETTINGS      = 0x03,

        // xctrl byte flags

        // Bit 7: xctrl override
        XCTRL_FLAG_ISXCTRL      = 0x80,
        // Bit 6: tuning enable
        XCTRL_FLAG_TUNING_EN    = 0x40,
        // Bit 5: fine tuning enable
        XCTRL_FLAG_FINE_EN      = 0x20,
        // Bit 4: unused
        // Bit 3: unused
        // Bit 2: panning enable
        XCTRL_FLAG_PANNING_EN   = 0x04,
        // Bits 0-1: panning setting
        XCTRL_FLAG_PANNING      = 0x03
    };

    Instrument();

    std::vector<uint8_t>& getProgram();

private:

    // instrument stream, sequence of commands
    // commands are applied on frame and have the format:
    // <ctrl> [<xctrl> [tune] [fine]] [aux1] [aux2]
    //  - OR -
    // <xctrl> [tune] [fine]
    std::vector<uint8_t> mStream;

};


}
