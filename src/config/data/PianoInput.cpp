
#include "config/data/PianoInput.hpp"

#include "config/data/keys.hpp"

#include <QKeySequence>

#include <algorithm>

#define TU PianoInputTU
namespace TU {

// reduced version of PianoInput::Bindings
struct StandardBinds {

    std::array<Qt::Key, 17> lowerBindings;
    std::array<Qt::Key, 20> upperBindings;
    std::array<Qt::Key, PianoInput::BindingCount - PianoInput::BindingNoteCut> specials;

};

// Standard QWERTY layout
// 1 2 3 4 5 6 7 8 9 0 - +  => CUT C#1 D#1 ... F#1 G#1 A#1 ... C#2 D#2 ... F#2
// Q W E R T Y U I O P [ ]  => C-1 D-1 E-1 F-1 G-1 A-1 B-1 C-2 D-2 E-2 F-2 G-2
// A S D F G H J K L ; ' \  => ... C#0 D#0 ... F#0 G#0 A#0 ... C#1 D#1 ... ...
// Z X C V B N M , . /      => C-0 D-0 E-0 F-0 G-0 A-0 B-0 C-1 D-1 E-1

// US QWERTY layout
static StandardBinds const QWERTY = {
    {
        // Lower row (ZXCVBN ... ASDFG ...)
        Qt::Key_Z,                  // C-0
        Qt::Key_S,                  // C#0
        Qt::Key_X,                  // D-0
        Qt::Key_D,                  // D#0
        Qt::Key_C,                  // E-0
        Qt::Key_V,                  // F-0
        Qt::Key_G,                  // F#0
        Qt::Key_B,                  // G-0
        Qt::Key_H,                  // G#0
        Qt::Key_N,                  // A-0
        Qt::Key_J,                  // A#0
        Qt::Key_M,                  // B-0
        Qt::Key_Comma,              // C-1
        Qt::Key_L,                  // C#1
        Qt::Key_Period,             // D-1
        Qt::Key_Semicolon,          // D#1
        Qt::Key_Slash,              // E-1
    },

    {
        // Upper row
        Qt::Key_Q,                  // C-1
        Qt::Key_2,                  // C#1
        Qt::Key_W,                  // D-1
        Qt::Key_3,                  // D#1
        Qt::Key_E,                  // E-1
        Qt::Key_R,                  // F-1
        Qt::Key_5,                  // F#1
        Qt::Key_T,                  // G-1
        Qt::Key_6,                  // G#1
        Qt::Key_Y,                  // A-1
        Qt::Key_7,                  // A#1
        Qt::Key_U,                  // B-1
        Qt::Key_I,                  // C-2
        Qt::Key_9,                  // C#2
        Qt::Key_O,                  // D-2
        Qt::Key_0,                  // D#2
        Qt::Key_P,                  // E-2
        Qt::Key_BracketLeft,        // F-2
        Qt::Key_Equal,              // F#2
        Qt::Key_BracketRight,       // G-2
    },

    {
        // Specials
        Qt::Key_1,                  // CUT
    }
};

// German T3 layout
static StandardBinds const QWERTZ = {
    {
        // Lower row
        Qt::Key_Y,                  // C-0
        Qt::Key_S,                  // C#0
        Qt::Key_X,                  // D-0
        Qt::Key_D,                  // D#0
        Qt::Key_C,                  // E-0
        Qt::Key_V,                  // F-0
        Qt::Key_G,                  // F#0
        Qt::Key_B,                  // G-0
        Qt::Key_H,                  // G#0
        Qt::Key_N,                  // A-0
        Qt::Key_J,                  // A#0
        Qt::Key_M,                  // B-0
        Qt::Key_Comma,              // C-1
        Qt::Key_L,                  // C#1
        Qt::Key_Period,             // D-1
        Qt::Key_Odiaeresis,         // D#1
        Qt::Key_Minus,              // E-1
    },

    {
        // Upper row
        Qt::Key_Q,                  // C-1
        Qt::Key_2,                  // C#1
        Qt::Key_W,                  // D-1
        Qt::Key_3,                  // D#1
        Qt::Key_E,                  // E-1
        Qt::Key_R,                  // F-1
        Qt::Key_5,                  // F#1
        Qt::Key_T,                  // G-1
        Qt::Key_6,                  // G#1
        Qt::Key_Z,                  // A-1
        Qt::Key_7,                  // A#1
        Qt::Key_U,                  // B-1
        Qt::Key_I,                  // C-2
        Qt::Key_9,                  // C#2
        Qt::Key_O,                  // D-2
        Qt::Key_0,                  // D#2
        Qt::Key_P,                  // E-2
        Qt::Key_Udiaeresis,         // F-2
        Qt::Key_Dead_Acute,         // F#2
        Qt::Key_Plus,               // G-2
    },

    {
        // Specials
        Qt::Key_1,                  // CUT
    }
};

// French AZERTY layout
static StandardBinds const AZERTY = {
    {
        // Lower row
        Qt::Key_W,                  // C-0
        Qt::Key_S,                  // C#0
        Qt::Key_X,                  // D-0
        Qt::Key_D,                  // D#0
        Qt::Key_C,                  // E-0
        Qt::Key_V,                  // F-0
        Qt::Key_G,                  // F#0
        Qt::Key_B,                  // G-0
        Qt::Key_H,                  // G#0
        Qt::Key_N,                  // A-0
        Qt::Key_J,                  // A#0
        Qt::Key_Comma,              // B-0
        Qt::Key_Semicolon,          // C-1
        Qt::Key_L,                  // C#1
        Qt::Key_Colon,              // D-1
        Qt::Key_M,                  // D#1
        Qt::Key_Exclam,             // E-1
    },

    {
        // Upper row
        Qt::Key_A,                  // C-1
        Qt::Key_Eacute,             // C#1
        Qt::Key_Z,                  // D-1
        Qt::Key_QuoteDbl,           // D#1
        Qt::Key_E,                  // E-1
        Qt::Key_R,                  // F-1
        Qt::Key_ParenLeft,          // F#1
        Qt::Key_T,                  // G-1
        Qt::Key_Minus,              // G#1
        Qt::Key_Y,                  // A-1
        Qt::Key_Egrave,             // A#1
        Qt::Key_U,                  // B-1
        Qt::Key_I,                  // C-2
        Qt::Key_Ccedilla,           // C#2
        Qt::Key_O,                  // D-2
        Qt::Key_Agrave,             // D#2
        Qt::Key_P,                  // E-2
        Qt::Key_Dead_Circumflex,    // F-2
        Qt::Key_Equal,              // F#2
        Qt::Key_Dollar,             // G-2
    },

    {
        // Specials
        Qt::Key_Ampersand,          // CUT
    }
};


}


PianoInput::PianoInput() :
    mOctave(3),
    mLayout(LayoutCustom),
    mBindings(),
    mBindingLookup()
{
}

PianoInput& PianoInput::operator=(PianoInput const& rhs) {
    mLayout = rhs.mLayout;
    mBindings = rhs.mBindings;
    mBindingLookup = rhs.mBindingLookup;
    
    return *this;
}


PianoInput::Bindings const& PianoInput::bindings() const {
    return mBindings;
}

void PianoInput::setBindings(Bindings const& bindings) {
    mLayout = LayoutCustom;
    mBindings = bindings;
    mapBindings();

}

void PianoInput::mapBindings() {
    mBindingLookup.clear();

    auto mapKey = [this](Qt::Key key, int semitone) {
        if (key != NoKey) {
            mBindingLookup.insert(key, semitone);
        }
    };

    auto iter = mBindings.begin();
    // lower and upper rows (lower octave starts at 0, upper at 1)
    for (auto semitone : {0, 12}) {
        for (int i = 0; i < BindsPerRow; ++i) {
            mapKey(*iter++, semitone++);
        }
    }
    // specials
    int special = SPECIAL_START;
    while (iter != mBindings.end()) {
        mapKey(*iter++, special++);
    }

}

PianoInput::KeyboardLayout PianoInput::layout() const {
    return mLayout;
}

void PianoInput::setLayout(KeyboardLayout layout) {
    if (layout == mLayout || layout >= LayoutCount) {
        return;
    }

    auto setBindings = [this](TU::StandardBinds const& binds) {

        auto iter = mBindings.begin();
        iter = std::copy(binds.lowerBindings.begin(), binds.lowerBindings.end(), iter);
        iter = std::fill_n(iter, BindsPerRow - binds.lowerBindings.size(), NoKey);
        iter = std::copy(binds.upperBindings.begin(), binds.upperBindings.end(), iter);
        iter = std::fill_n(iter, BindsPerRow - binds.upperBindings.size(), NoKey);
        std::copy(binds.specials.begin(), binds.specials.end(), iter);


        mapBindings();
    };

    switch (layout) {
        case LayoutQwerty:
            setBindings(TU::QWERTY);
            break;
        case LayoutQwertz:
            setBindings(TU::QWERTZ);
            break;
        case LayoutAzerty:
            setBindings(TU::AZERTY);
            break;
        default:
            break;
    }

    mLayout = layout;
}

std::optional<trackerboy::Note> PianoInput::keyToNote(int key) const {

    if (mBindingLookup.contains(key)) {
        auto offset = mBindingLookup[key];
        if (offset == SPECIAL_START) {
            // since there's only one special (note cut) we can just check if the offset == SPECIAL_START
            // more specials will require a switch/lookup table
            return trackerboy::NOTE_CUT;
        } else {
            int note = mOctave * 12 + offset;
            if (note <= trackerboy::NOTE_LAST) {
                return static_cast<trackerboy::Note>(note);
            }
        }
    }

    return std::nullopt;

}

int PianoInput::octave() const {
    return mOctave + 2;
}

void PianoInput::setOctave(int octave) {
    if (octave >= 2 && octave <= 8) {
        mOctave = octave - 2;
    }
}


// configuration

namespace TU {

static std::array const LAYOUT_KEYS = {
    QStringLiteral("qwerty"),
    QStringLiteral("qwertz"),
    QStringLiteral("azerty"),
    QStringLiteral("custom")
};
static_assert(LAYOUT_KEYS.size() == PianoInput::LayoutCount, "missing key for a layout");


static Qt::Key readBinding(QSettings &settings, QString const& keyname) {
    QKeySequence seq(settings.value(keyname).toString(), QKeySequence::PortableText);
    if (seq.isEmpty()) {
        return PianoInput::NoKey;
    } else {
        return (Qt::Key)seq[0];
    }
}

static void writeBinding(QSettings &settings, QString const& keyname, Qt::Key key) {
    settings.setValue(keyname, QKeySequence(key).toString(QKeySequence::PortableText));
}

template <class Iter>
static Iter readRowBindings(QSettings &settings, Iter dest, QString const& key) {
    auto count = settings.beginReadArray(key);
    if (count == PianoInput::BindsPerRow) {
        for (int i = 0; i < count; ++i) {
            settings.setArrayIndex(i);
            *dest++ = TU::readBinding(settings, Keys::key);

        }
    }
    settings.endArray();
    return dest;
}

template <class Iter>
static Iter writeRowBindings(QSettings &settings, Iter src, QString const& key) {
    settings.beginWriteArray(key, PianoInput::BindsPerRow);
    for (int i = 0; i < PianoInput::BindsPerRow; ++i) {
        settings.setArrayIndex(i);
        writeBinding(settings, Keys::key, *src++);
    }
    settings.endArray();
    return src;
}


}

QString PianoInput::layoutName(KeyboardLayout layout) {
    switch (layout) {
        case LayoutQwerty:
        case LayoutQwertz:
        case LayoutAzerty:
            return TU::LAYOUT_KEYS[layout].toUpper();
        case LayoutCustom:
            return tr("Custom");
        default:
            return {};
    }
}



void PianoInput::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::PianoInput);

    auto const layoutStr = settings.value(Keys::keyboardLayout).toString();
    auto layoutEnum = [](QString const& str) {
        if (!str.isEmpty()) {
            int result = 0;
            for (auto const &key : TU::LAYOUT_KEYS) {
                if (str == key) {
                    return (KeyboardLayout)result;
                }
                result++;
            }
        }

        // default to QWERTY
        return LayoutQwerty;
    }(layoutStr);

    if (layoutEnum == LayoutCustom) {

        auto iter = mBindings.begin();
        iter = TU::readRowBindings(settings, iter, Keys::bindingsLower);
        TU::readRowBindings(settings, iter, Keys::bindingsUpper);

        // specials
        mBindings[BindingNoteCut] = TU::readBinding(settings, Keys::noteCut);

        mapBindings();
    } else {
        setLayout(layoutEnum);
    }

    settings.endGroup();
}

void PianoInput::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::PianoInput);
    settings.remove(QString());

    settings.setValue(Keys::keyboardLayout, TU::LAYOUT_KEYS[mLayout]);

    if (mLayout == LayoutCustom) {

        auto iter = mBindings.begin();
        iter = TU::writeRowBindings(settings, iter, Keys::bindingsLower);
        TU::writeRowBindings(settings, iter, Keys::bindingsUpper);

        // specials
        TU::writeBinding(settings, Keys::noteCut, mBindings[BindingNoteCut]);
    }


    settings.endGroup();
}




#undef TU
