
#include "core/PianoInput.hpp"

#include <algorithm>

PianoInput::PianoInput() :
    mOctave(3)
{
    mBindings.fill(Qt::Key_unknown);
}

PianoInput& PianoInput::operator=(PianoInput const& rhs) {
    std::copy(rhs.mBindings.cbegin(), rhs.mBindings.cend(), mBindings.begin());
    mBindingLookup = rhs.mBindingLookup;
    return *this;
}

PianoInput::Bindings const& PianoInput::bindings() const {
    return mBindings;
}

void PianoInput::bind(int key, int semitoneOffset) {
    mBindings[semitoneOffset] = key;
    mBindingLookup[key] = semitoneOffset;
}

void PianoInput::unbind(int semitoneOffset) {
    auto &key = mBindings[semitoneOffset]; 
    mBindingLookup.remove(key);
    key = Qt::Key_unknown;
    
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

void PianoInput::setOctave(int octave) {
    if (octave >= 2 && octave <= 8) {
        mOctave = octave - 2;
    }
}
