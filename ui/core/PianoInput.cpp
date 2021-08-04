
#include "core/PianoInput.hpp"

#include "core/config/keys.hpp"

#include <QKeySequence>

#include <algorithm>

PianoInput::PianoInput() :
    mOctave(3),
    mLayout(LayoutCustom),
    mBindings(),
    mBindingLookup()
{
    clearBindings();
}

PianoInput& PianoInput::operator=(PianoInput const& rhs) {
    std::copy(rhs.mBindings.cbegin(), rhs.mBindings.cend(), mBindings.begin());
    mBindingLookup = rhs.mBindingLookup;
    return *this;
}

PianoInput::Bindings const& PianoInput::bindings() const {
    return mBindings;
}

PianoInput::KeyboardLayout PianoInput::layout() const {
    return mLayout;
}

void PianoInput::setLayout(KeyboardLayout layout) {
    if (layout == mLayout) {
        return;
    }

    if (layout != LayoutCustom) {
        clearBindings();

        int keyQ = Qt::Key_Q;
        int keyW = Qt::Key_W;
        int keyY = Qt::Key_Y;
        int keyZ = Qt::Key_Z;
        int keyA = Qt::Key_A;

        if (layout == LayoutQwertz) {
            // A and Z are swapped on QWERTZ
            std::swap(keyY, keyZ);
        } else if (layout == LayoutAzerty) {
            // A and Q, Z and W are swapped on AZERTY
            std::swap(keyA, keyQ);
            std::swap(keyZ, keyW);
        }
        
        constexpr auto OCTAVE_0 = 0;
        constexpr auto OCTAVE_1 = 12;
        constexpr auto OCTAVE_2 = 24;
        
// Standard QWERTY layout
// 1 2 3 4 5 6 7 8 9 0 - +  => CUT C#1 D#1 ... F#1 G#1 A#1 ... C#2 D#2 ... F#2
// Q W E R T Y U I O P [ ]  => C-1 D-1 E-1 F-1 G-1 A-1 B-1 C-2 D-2 E-2 F-2 G-2
// A S D F G H J K L ; ' \  => ... C#0 D#0 ... F#0 G#0 A#0 ... C#1 D#1 ... ...
// Z X C V B N M , . /      => C-0 D-0 E-0 F-0 G-0 A-0 B-0 C-1 D-1 E-1

        // populate standard bindings assuming QWERTY
        // Row 0 (numbers)
        bind(Qt::Key_1,             NOTE_CUT_BINDING);
        bind(Qt::Key_2,             trackerboy::NOTE_Db + OCTAVE_1);
        bind(Qt::Key_3,             trackerboy::NOTE_Eb + OCTAVE_1);
        bind(Qt::Key_5,             trackerboy::NOTE_Gb + OCTAVE_1);
        bind(Qt::Key_6,             trackerboy::NOTE_Ab + OCTAVE_1);
        bind(Qt::Key_7,             trackerboy::NOTE_Bb + OCTAVE_1);
        bind(Qt::Key_9,             trackerboy::NOTE_Db + OCTAVE_2);
        bind(Qt::Key_0,             trackerboy::NOTE_Eb + OCTAVE_2);
        bind(Qt::Key_Equal,         trackerboy::NOTE_Gb + OCTAVE_2);
        // Row 1 (QWERTY)
        bind(keyQ,                  trackerboy::NOTE_C  + OCTAVE_1);
        bind(keyW,                  trackerboy::NOTE_D  + OCTAVE_1);
        bind(Qt::Key_E,             trackerboy::NOTE_E  + OCTAVE_1);
        bind(Qt::Key_R,             trackerboy::NOTE_F  + OCTAVE_1);
        bind(Qt::Key_T,             trackerboy::NOTE_G  + OCTAVE_1);
        bind(keyY,                  trackerboy::NOTE_A  + OCTAVE_1);
        bind(Qt::Key_U,             trackerboy::NOTE_B  + OCTAVE_1);
        bind(Qt::Key_I,             trackerboy::NOTE_C  + OCTAVE_2);
        bind(Qt::Key_O,             trackerboy::NOTE_D  + OCTAVE_2);
        bind(Qt::Key_P,             trackerboy::NOTE_E  + OCTAVE_2);
        bind(Qt::Key_BracketLeft,   trackerboy::NOTE_F  + OCTAVE_2);
        bind(Qt::Key_BracketRight,  trackerboy::NOTE_G  + OCTAVE_2);
        // Row 2 (ASDFGH)
        bind(Qt::Key_S,             trackerboy::NOTE_Db + OCTAVE_0);
        bind(Qt::Key_D,             trackerboy::NOTE_Eb + OCTAVE_0);
        bind(Qt::Key_G,             trackerboy::NOTE_Gb + OCTAVE_0);
        bind(Qt::Key_H,             trackerboy::NOTE_Ab + OCTAVE_0);
        bind(Qt::Key_J,             trackerboy::NOTE_Bb + OCTAVE_0);
        bind(Qt::Key_L,             trackerboy::NOTE_Db + OCTAVE_1);
        bind(Qt::Key_Semicolon,     trackerboy::NOTE_Eb + OCTAVE_1);
        // Row 3 (ZXCVBN)
        bind(keyZ,                  trackerboy::NOTE_C  + OCTAVE_0);
        bind(Qt::Key_X,             trackerboy::NOTE_D  + OCTAVE_0);
        bind(Qt::Key_C,             trackerboy::NOTE_E  + OCTAVE_0);
        bind(Qt::Key_V,             trackerboy::NOTE_F  + OCTAVE_0);
        bind(Qt::Key_B,             trackerboy::NOTE_G  + OCTAVE_0);
        bind(Qt::Key_N,             trackerboy::NOTE_A  + OCTAVE_0);
        bind(Qt::Key_M,             trackerboy::NOTE_B  + OCTAVE_0);
        bind(Qt::Key_Comma,         trackerboy::NOTE_C  + OCTAVE_1);
        bind(Qt::Key_Period,        trackerboy::NOTE_D  + OCTAVE_1);
        bind(Qt::Key_Slash,         trackerboy::NOTE_E  + OCTAVE_1);
        

    }
    


    mLayout = layout;


}

void PianoInput::bind(int key, int semitoneOffset) {
    mBindings[semitoneOffset] = key;
    mBindingLookup[key] = semitoneOffset;
    mLayout = LayoutCustom;
}

void PianoInput::unbind(int semitoneOffset) {
    auto &key = mBindings[semitoneOffset]; 
    mBindingLookup.remove(key);
    key = Qt::Key_unknown;
    mLayout = LayoutCustom;
}

void PianoInput::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::PianoInput);

    auto layout = settings.value(Keys::keyboardLayout, LayoutQwerty).toInt();
    if (layout < LayoutQwerty || layout > LayoutCustom) {
        layout = LayoutQwerty; // default to QWERTY
    }
    setLayout((KeyboardLayout)layout);

    if (layout == LayoutCustom) {
        // only custom layouts have bindings saved
        clearBindings();
        auto bindings = settings.beginReadArray(Keys::bindings);
        if (bindings == TOTAL_BINDINGS) {
            for (int i = 0; i < bindings; ++i) {
                settings.setArrayIndex(i);
                auto keyname = settings.value(Keys::key).toString();
                if (!keyname.isEmpty()) {
                    QKeySequence seq(keyname);
                    if (seq.count() == 1) {
                        bind(seq[0], i);
                    }

                }
            }
        }
        settings.endArray();
    }

    settings.endGroup();
}

void PianoInput::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::PianoInput);
    settings.remove(QString());

    settings.setValue(Keys::keyboardLayout, mLayout);
    if (mLayout == LayoutCustom) {
        settings.beginWriteArray(Keys::bindings, TOTAL_BINDINGS);
        for (int i = 0; i < TOTAL_BINDINGS; ++i) {
            settings.setArrayIndex(i);
            
            auto key = mBindings[i];
            QString keyName;
            if (key != Qt::Key_unknown) {
                keyName = QKeySequence(key).toString();
            }

            settings.setValue(Keys::key, keyName);
        }
        settings.endArray();
    }


    settings.endGroup();
}


std::optional<trackerboy::Note> PianoInput::keyToNote(int key) const {

    if (mBindingLookup.contains(key)) {
        auto offset = mBindingLookup[key];
        if (offset == NOTE_CUT_BINDING) {
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

void PianoInput::clearBindings() {
    mBindings.fill(Qt::Key_unknown);
    mBindingLookup.clear();
}
