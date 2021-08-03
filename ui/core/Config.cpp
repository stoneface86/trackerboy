
#include "core/Config.hpp"
#include "core/samplerates.hpp"

#include "gbapu.hpp"

#include "core/audio/AudioProber.hpp"
#include "core/midi/MidiProber.hpp"

#include "RtMidi.h"

#include <QSettings>
#include <QtDebug>

namespace {


Qt::Key const DEFAULT_PIANO_BINDINGS[] = {

    // base octave
    Qt::Key_Q,              // C
    Qt::Key_W,
    Qt::Key_E,
    Qt::Key_R,
    Qt::Key_T,
    Qt::Key_Y,
    Qt::Key_U,
    Qt::Key_I,
    Qt::Key_O,
    Qt::Key_P,
    Qt::Key_BracketLeft,
    Qt::Key_BracketRight,   // B

    // base octave + 1

    Qt::Key_A,              // C
    Qt::Key_S,
    Qt::Key_D,
    Qt::Key_F,
    Qt::Key_G,
    Qt::Key_H,
    Qt::Key_J,
    Qt::Key_K,
    Qt::Key_L,
    Qt::Key_Semicolon,
    Qt::Key_Apostrophe,
    Qt::Key_Backslash,      // B

    // base octave + 2

    Qt::Key_Z,              // C
    Qt::Key_X,
    Qt::Key_C,
    Qt::Key_V,
    Qt::Key_B,
    Qt::Key_N,
    Qt::Key_M,
    Qt::Key_Comma,
    Qt::Key_Period,
    Qt::Key_Slash,          // A
    Qt::Key_unknown,
    Qt::Key_unknown,
    Qt::Key_QuoteLeft      // note cut
};

}

Config::Config() :
    mAppearance(),
    mGeneral(),
    //mKeyboard(),
    mMidi(),
    mSound(),
    mPalette()
{
}

AppearanceConfig& Config::appearance() { return mAppearance; }
AppearanceConfig const& Config::appearance() const { return mAppearance; }

GeneralConfig& Config::general() { return mGeneral; }
GeneralConfig const& Config::general() const { return mGeneral; }

// KeyboardConfig& Config::keyboard() { return mKeyboard; }
// KeyboardConfig const& Config::keyboard() const { return mKeyboard; }

MidiConfig& Config::midi()                  { return mMidi; }
MidiConfig const& Config::midi() const      { return mMidi; }

SoundConfig& Config::sound()                { return mSound; }
SoundConfig const& Config::sound() const    { return mSound; }

Palette& Config::palette() { return mPalette; }
Palette const& Config::palette() const { return mPalette; }

void Config::readSettings() {
    QSettings settings;

    mAppearance.readSettings(settings);
    mGeneral.readSettings(settings);
    mMidi.readSettings(settings);
    mSound.readSettings(settings);
    mPalette.readSettings(settings);
}


void Config::writeSettings() {
    QSettings settings;

    mAppearance.writeSettings(settings);
    mGeneral.writeSettings(settings);
    mMidi.writeSettings(settings);
    mSound.writeSettings(settings);
    mPalette.writeSettings(settings);
}

void Config::disableMidi() {
    mMidi.setEnabled(false);
}
