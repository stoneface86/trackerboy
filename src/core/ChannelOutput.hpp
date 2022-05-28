
#pragma once

#include "verdigris/wobjectdefs.h"

#include <QFlags>

namespace ChannelOutput {

    //
    // These flags determine which channels are enabled for music playback
    // Typically used for solo'ing a channel by disabling all channels except
    // for one.
    //
    enum Flag {
        AllOff = 0x0,
        CH1 = 0x1,
        CH2 = 0x2,
        CH3 = 0x4,
        CH4 = 0x8,
        AllOn = CH1 | CH2 | CH3 | CH4
    };
    W_DECLARE_FLAGS(Flags, Flag)

}

Q_DECLARE_OPERATORS_FOR_FLAGS(ChannelOutput::Flags)
W_REGISTER_ARGTYPE(ChannelOutput::Flags)
W_REGISTER_ARGTYPE(ChannelOutput::Flag)
