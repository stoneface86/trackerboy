
#include "trackerboy/instrument.hpp"

#define CHAR_NOT_HEX 255


namespace trackerboy {


uint8_t hextoint(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    } else {
        return CHAR_NOT_HEX;
    }
}

uint8_t toNote(std::string str) {
    static const Note NOTE_LOOKUP[] = {
        NOTE_A, // ('A' - 'A') -> A2
        NOTE_B,
        NOTE_C,
        NOTE_D,
        NOTE_E,
        NOTE_F,
        NOTE_G
    };

    // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
    // C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B

    // #: C, D, F, G, A

    // assumption: str is uppercase

    // format: <note><#|-><octave>

    if (str.length() != 3) {
        return NOTE_NONE;
    }

    int note = str[0] - 'A';
    if (note < 0 || note >= sizeof(NOTE_LOOKUP)) {
        return NOTE_NONE;
    }

    uint8_t result = NOTE_LOOKUP[note];
    if (str[1] == '#') {
        if (str[0] == 'B' || str[0] == 'E') {
            return NOTE_NONE;
        }
        ++result;
    } else if (str[1] != '-') {
        return NOTE_NONE;
    }

    int octave = str[2] - '2';
    if (octave < 0 || octave >= 6) {
        return NOTE_NONE;
    }

    return result + (octave * 12);
}

//
// Split a string using whitespace as a delimiter
//
std::vector<std::string> split(std::string str, std::string delims = "", size_t maxsplit = 0) {
    std::vector<std::string> strVec;
    static const std::string whitespace = "\t\r\n ";
    if (delims == "") {
        delims = whitespace;
    }
    
    size_t splitcount = 0;
    size_t pos = 0;
    size_t start;
    while (pos < str.length()) {
        start = str.find_first_not_of(delims, pos);
        if (start == std::string::npos) {
            break;
        }
        pos = str.find_first_of(delims, start);
        if (pos == std::string::npos) {
            // no delimiter found
            strVec.push_back(str.substr(start));
            break;
        }
        strVec.push_back(str.substr(start, pos - start));
        if (++splitcount == maxsplit) {
            strVec.push_back(str.substr(pos + 1));
            break;
        }
    }

    return strVec;
}


static Instruction _parsePulseSettings(ChType trackId, std::vector<std::string> settings) {
    Instruction inst = { 0 };

    uint8_t dutySet = 0;
    uint8_t duty = 0;

    uint8_t sweepSet = 0;
    uint8_t sweepShift = 0;
    uint8_t sweepMode = 0;
    uint8_t sweepTime = 0;


    std::string token;
    for (auto iter = settings.begin(); iter != settings.end(); ++iter) {
        token = *iter;
        switch (toupper(token[0])) {
            case 'D':
                if (token.length() != 2) {
                    throw ParseError(0);
                }
                dutySet = Instruction::CTRL_SET_DUTY;
                duty = hextoint(token[1]);
                if (duty == CHAR_NOT_HEX || duty > 3) {
                    throw ParseError(0);
                }
                break;
            case 'S':
                if (trackId != ChType::ch1 || token.length() != 4) {
                    throw ParseError(0);
                } else {  // else used for scope
                    sweepSet = Instruction::SETTINGS_SET_SWEEP;
                    sweepTime = hextoint(token[1]);
                    if (sweepTime == CHAR_NOT_HEX || sweepTime > Gbs::MAX_SWEEP_TIME) {
                        throw ParseError(0);
                    }
                    char modechar = token[2];
                    if (modechar == '+') {
                        // sweep addition
                        sweepMode = static_cast<uint8_t>(Gbs::SWEEP_ADDITION);
                    } else if (modechar == '-') {
                        sweepMode = static_cast<uint8_t>(Gbs::SWEEP_SUBTRACTION);
                    } else {
                        throw ParseError(0);
                    }
                    sweepShift = hextoint(token[3]);
                    if (sweepShift == CHAR_NOT_HEX || sweepShift > Gbs::MAX_SWEEP_SHIFT) {
                        throw ParseError(0);
                    }
                }
                break;
            default:
                // raise error;
                break;
        }
    }

    if (trackId == ChType::ch1) {
        inst.settings = sweepSet | (sweepTime << 4) | (sweepMode << 3) | sweepShift;
    }
    inst.ctrl = dutySet | duty;

    return inst;
}


static Instruction _parseWaveSettings(ChType trackId, std::vector<std::string> settings) {
    Instruction inst = { 0 };

    // TODO: implement wave parser

    return inst;
}


static Instruction _parseNoiseSettings(ChType trackId, std::vector<std::string> settings) {
    Instruction inst = { 0 };

    // TODO: implement noise parser

    return inst;
}



Instruction parse(ChType trackId, std::string line) {
    // format: [duration:] setting1 [setting2] [setting3]...
    Instruction inst;
    inst.duration = 1; // default to 1 if duration is omitted from line

    // convert line to uppercase
    for (auto iter = line.begin(); iter != line.end(); ++iter) {
        *iter = toupper(*iter);
    }

    auto tokens = split(line, ":", 1);
    if (tokens.size() == 2) {
        // parse the frame duration
        size_t duration;
        try {
            duration = std::stoul(tokens[0]);
        } catch (std::invalid_argument &ia) {
            throw ParseError(0);
        } catch (std::out_of_range &oor) {
            throw ParseError(0);
        }
        if (duration > UINT8_MAX || duration == 0) {
            throw ParseError(0);
        }
        inst.duration = static_cast<uint8_t>(duration);
    }

    // split the tokens using whitespace as a delimiter
    tokens = split(*(--tokens.end()));
    auto tokensIter = tokens.begin();

    // parse settings that apply to all channels, then push the remaining into
    // a separate vector

    Instruction::Panning panning = Instruction::PANNING_NOSET;
    Instruction::EnvCtrl envCtrl = Instruction::ENV_NOSET;
    uint8_t init = 0;

    // env
    uint8_t envVolume = 0;
    uint8_t envMode = 0;
    uint8_t envLength = 0;


    std::vector<std::string> settings;
    std::string token;
    while (tokensIter != tokens.end()) {
        token = *tokensIter;
        switch (token[0]) {
            case 'E':          // envelope
            {
                if (trackId == ChType::ch3) {
                    throw ParseError(0);
                }
                size_t offset;
                if (token.length() == 4) {
                    // envelope with volume
                    envCtrl = Instruction::ENV_SETVOL;
                    envVolume = hextoint(token[1]);
                    offset = 1;
                    if (envVolume == CHAR_NOT_HEX) {
                        throw ParseError(0);
                    }

                } else if (token.length() == 3) {
                    // envelope without volume (from note)
                    envCtrl = Instruction::ENV_SETNOVOL;
                    envVolume = 0; // the volume will be set from the row volume when played
                    offset = 0;
                } else {
                    throw ParseError(0);
                }

                char modechar = token[offset + 1];
                if (modechar == '/') {
                    envMode = static_cast<uint8_t>(Gbs::ENV_AMPLIFY);
                } else if (modechar == '\\') {
                    envMode = static_cast<uint8_t>(Gbs::ENV_ATTENUATE);
                } else {
                    throw ParseError(0);
                }

                envLength = hextoint(token[offset + 2]);
                if (envLength == CHAR_NOT_HEX || envLength > 7) {
                    throw ParseError(0);
                }

                break;
            }
            case 'I':           // init
                if (token.length() == 1) {
                    inst.note = NOTE_NONE;
                } else {
                    uint8_t note = toNote(token.substr(1));
                    if (note == NOTE_NONE) {
                        throw ParseError(0);
                    }
                    inst.note = note;
                }
                init = Instruction::CTRL_INIT;
                break;
            case 'P':           // panning
                if (token.length() != 2) {
                    throw ParseError(0);
                }
                switch (toupper(token[1])) {
                    case 'L':
                        panning = Instruction::PANNING_LEFT;
                        break;
                    case 'M':
                        panning = Instruction::PANNING_BOTH;
                        break;
                    case 'R':
                        panning = Instruction::PANNING_RIGHT;
                        break;
                    default:
                        throw ParseError(0);
                }
                break;
            default:
                settings.push_back(token);
                break;
        }
        ++tokensIter;
    }

    inst.ctrl = init | envCtrl | panning;
    if (trackId != ChType::ch3) {
        inst.envSettings = (envVolume << 4) | (envMode << 3) | envLength;
    }

    // parse channel-specific settings
    Instruction channelInst;
    
    switch (trackId) {
        case ChType::ch1:
        case ChType::ch2:
            channelInst = _parsePulseSettings(trackId, settings);
            break;
        case ChType::ch3:
            channelInst = _parseWaveSettings(trackId, settings);
            break;
        case ChType::ch4:
            channelInst = _parseNoiseSettings(trackId, settings);
            break;
    }
    
    inst.ctrl |= channelInst.ctrl;            // bits 0-4 of ctrl are channel-specific
    inst.settings = channelInst.settings;

    return inst;
}


}
