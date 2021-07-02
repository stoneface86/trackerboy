
#include "core/Config.hpp"

#include "gbapu.hpp"

#include "core/audio/AudioProber.hpp"
#include "core/midi/MidiProber.hpp"

#include "rtaudio/RtAudio.h"
#include "RtMidi.h"

#include <QSettings>
#include <QtDebug>


namespace {

constexpr int DEFAULT_SAMPLERATE_INDEX = 4; // 44100 Hz
constexpr int DEFAULT_PERIOD = 5;
constexpr int DEFAULT_LATENCY = 40;
constexpr int DEFAULT_QUALITY = static_cast<int>(gbapu::Apu::Quality::medium);
constexpr unsigned DEFAULT_HISTORY_LIMIT = 64;

constexpr int DEFAULT_MIDI_API = RtMidi::UNSPECIFIED;


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
    mKeyboard(),
    mMidi(),
    mSound()
{
}

Config::Appearance const& Config::appearance() const {
    return mAppearance;
}

Config::General const& Config::general() const {
    return mGeneral;
}

Config::Keyboard const& Config::keyboard() const {
    return mKeyboard;
}

Config::Midi const& Config::midi() const {
    return mMidi;
}

Config::Sound const& Config::sound() const {
    return mSound;
}

void Config::readSettings() {
    QSettings settings;


    settings.beginGroup("config");

    settings.beginGroup(QStringLiteral("appearance"));

    mAppearance.font.setFamily(settings.value("fontFamily", "Cascadia Mono").toString());
    mAppearance.font.setPointSize(settings.value("fontSize", 12).toInt());

    settings.beginReadArray("colors");
    readColor(settings, Color::background,              QColor(0x18, 0x18, 0x18));
    readColor(settings, Color::backgroundHighlight1,    QColor(0x20, 0x20, 0x20));
    readColor(settings, Color::backgroundHighlight2,    QColor(0x30, 0x30, 0x30));
    readColor(settings, Color::foreground,              QColor(0xC0, 0xC0, 0xC0));
    readColor(settings, Color::foregroundHighlight1,    QColor(0xF0, 0xF0, 0xF0));
    readColor(settings, Color::foregroundHighlight2,    QColor(0xFF, 0xFF, 0xFF));
    readColor(settings, Color::row,                     QColor(0x20, 0x30, 0x80));
    readColor(settings, Color::rowEdit,                 QColor(0x80, 0x20, 0x20));
    readColor(settings, Color::rowPlayer,               QColor(0x80, 0x20, 0x80));
    readColor(settings, Color::effectType,              QColor(0x80, 0x80, 0xFF));
    readColor(settings, Color::instrument,              QColor(0x80, 0xFF, 0x80));
    readColor(settings, Color::selection,               QColor(0x45, 0x45, 0x50));
    readColor(settings, Color::cursor,                  QColor(192, 192, 192));
    readColor(settings, Color::line,                    QColor(64, 64, 64));
    readColor(settings, Color::headerBackground,        QColor(224, 248, 208));
    readColor(settings, Color::headerForeground,        QColor(8, 24, 32));
    readColor(settings, Color::headerHover,             QColor(136, 192, 112));
    readColor(settings, Color::headerDisabled,          QColor(52, 104, 86));
    settings.endArray();
   
    mAppearance.showFlats = settings.value("showFlats", false).toBool();
    mAppearance.showPreviews = settings.value("showPreviews", true).toBool();

    settings.endGroup(); // appearance

    settings.beginGroup(QStringLiteral("general"));
    mGeneral.historyLimit = settings.value(QStringLiteral("historyLimit"), DEFAULT_HISTORY_LIMIT).toUInt();
    settings.endGroup(); // general

    settings.beginGroup(QStringLiteral("keyboard"));

    // keybindings for the piano widget + note column in pattern editor
    settings.beginReadArray("piano");
    for (int i = 0; i != sizeof(DEFAULT_PIANO_BINDINGS) / sizeof(Qt::Key); ++i) {
        settings.setArrayIndex(i);
        mKeyboard.pianoInput.bind(
            static_cast<Qt::Key>(settings.value("keycode", DEFAULT_PIANO_BINDINGS[i]).toInt()),
            i
        );
    }
    settings.endArray();

    settings.endGroup(); // keyboard


    settings.beginGroup(QStringLiteral("midi"));

    mMidi.enabled = settings.value(QStringLiteral("enabled"), false).toBool();
    auto api = (RtMidi::Api)settings.value(QStringLiteral("api"), DEFAULT_MIDI_API).toInt();
    auto portName = settings.value(QStringLiteral("portname"), QString()).toString();

    {
        auto &prober = MidiProber::instance();
        if (api == RtMidi::UNSPECIFIED) {
            mMidi.backendIndex = 0; // default to first available
        } else {
            mMidi.backendIndex = prober.indexOfApi(api);
            if (mMidi.backendIndex == -1) {
                // this should rarely happen
                qWarning() << "MIDI API is not available, falling back to the first available";
                mMidi.backendIndex = 0; // default to the first one
            }
        }
        prober.setBackend(mMidi.backendIndex);
        
        if (portName.isEmpty()) {
            mMidi.portIndex = -1;
        } else {
            // we don't really have a way to uniquely identify the device so just use the
            // port name.
            mMidi.portIndex = prober.portNames().indexOf(portName);
            // if portIndex is -1, then we couldn't find the port, log a warning
            if (mMidi.portIndex == -1) {
                qWarning() << "Could not find MIDI port, please select a new device";
            }
        }
        
    }


    settings.endGroup(); // midi



    settings.beginGroup(QStringLiteral("sound"));
    {
        auto &audioProber = AudioProber::instance();
        auto const api = (RtAudio::Api)settings.value(QStringLiteral("api"), RtAudio::UNSPECIFIED).toInt();
        mSound.deviceName = settings.value(QStringLiteral("deviceName"), QString()).toString();
        if (api == RtAudio::UNSPECIFIED) {
            mSound.backendIndex = 0; // default to the first one
            audioProber.probe(0);
        } else {
            mSound.backendIndex = audioProber.indexOfApi(api);
            if (mSound.backendIndex == -1) {
                auto apiName = RtAudio::getApiName(api);
                qWarning().noquote() << "audio API" << QString::fromStdString(apiName) << "not available";
            } else {
                audioProber.probe(mSound.backendIndex);
            }
        }
        if (mSound.deviceName.isEmpty()) {
            mSound.deviceIndex = audioProber.getDefaultDevice(mSound.backendIndex);
        } else {
            mSound.deviceIndex = audioProber.indexOfDevice(mSound.backendIndex, mSound.deviceName);
            if (mSound.deviceIndex == -1) {
                qWarning().noquote() << "audio device" << mSound.deviceName << "not available";
            }
        }
    }

    mSound.samplerateIndex = settings.value("samplerateIndex", DEFAULT_SAMPLERATE_INDEX).toInt();
    mSound.latency = settings.value("latency", DEFAULT_LATENCY).toInt();
    mSound.period = settings.value("period", DEFAULT_PERIOD).toInt();
    mSound.quality = settings.value("quality", DEFAULT_QUALITY).toInt();

    settings.endGroup(); // sound

    settings.endGroup(); // config
}

void Config::readColor(QSettings &settings, Color color, QColor def) {
    settings.setArrayIndex(+color);
    mAppearance.colors[+color] = settings.value("color", def).value<QColor>();
}


void Config::writeSettings() {
    QSettings settings;
    settings.beginGroup("config");

    settings.beginGroup("appearance");
    settings.setValue("fontFamily", mAppearance.font.family());
    settings.setValue("fontSize", mAppearance.font.pointSize());
    settings.beginWriteArray("colors", (int)mAppearance.colors.size());
    for (size_t i = 0; i != mAppearance.colors.size(); ++i) {
        settings.setArrayIndex((int)i);
        settings.setValue("color", mAppearance.colors[i]);
    }
    settings.endArray();
    settings.setValue("showFlats", mAppearance.showFlats);
    settings.setValue("showPreviews", mAppearance.showPreviews);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("general"));
    settings.setValue("historyLimit", mGeneral.historyLimit);
    settings.endGroup();

    settings.beginGroup("keyboard");
    settings.beginWriteArray("piano");
    auto &bindings = mKeyboard.pianoInput.bindings();
    for (size_t i = 0; i != bindings.size(); ++i) {
        settings.setArrayIndex((int)i);
        settings.setValue("keycode", bindings[i]);
    }
    settings.endArray();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("midi"));
    {
        settings.setValue(QStringLiteral("enabled"), mMidi.enabled);
        auto &prober = MidiProber::instance();
        settings.setValue(QStringLiteral("api"), (int)prober.api());
        QString str;
        if (mMidi.portIndex != -1) {
            str = prober.portNames().at(mMidi.portIndex);
        }
        settings.setValue(QStringLiteral("portname"), str);
    }
    settings.endGroup();

    settings.beginGroup("sound");

    {
        auto &audioProber = AudioProber::instance();
        settings.setValue(QStringLiteral("api"), (int)audioProber.backendApi(mSound.backendIndex));
        settings.setValue(QStringLiteral("deviceName"), mSound.deviceName);
    }

    settings.setValue("samplerateIndex", mSound.samplerateIndex);
    settings.setValue("latency", mSound.latency);
    settings.setValue("period", mSound.period);
    settings.setValue("quality", mSound.quality);
    settings.endGroup();

    settings.endGroup();
}

void Config::disableMidi() {
    mMidi.enabled = false;
}
