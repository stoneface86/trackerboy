
#include "core/Config.hpp"
#include "core/samplerates.hpp"

#include "gbapu.hpp"

#include "core/audio/AudioProber.hpp"
#include "core/midi/MidiProber.hpp"

#include "RtMidi.h"

#include <QSettings>
#include <QtDebug>

namespace {

static auto LOG_PREFIX = "[Config]";


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


// QString interning for QSettings keys
// if we use c strings then the space needed will be halved
// but an implicit conversion to QString is required at runtime
namespace Keys {

// keep in alphabetical order
static auto const APPEARANCE        = QStringLiteral("appearance");
static auto const API               = QStringLiteral("api");
static auto const BACKEND           = QStringLiteral("backend");
static auto const COLOR             = QStringLiteral("color");
static auto const COLORS            = QStringLiteral("colors");
static auto const CONFIG            = QStringLiteral("config");
static auto const DEVICE_ID         = QStringLiteral("deviceId");
static auto const ENABLED           = QStringLiteral("enabled");
static auto const FONT_FAMILY       = QStringLiteral("fontFamily");
static auto const FONT_SIZE         = QStringLiteral("fontSize");
static auto const GENERAL           = QStringLiteral("general");
static auto const HISTORY_LIMIT     = QStringLiteral("historyLimit");
static auto const KEYBOARD          = QStringLiteral("keyboard");
static auto const KEYCODE           = QStringLiteral("keycode");
static auto const LATENCY           = QStringLiteral("latency");
static auto const MIDI              = QStringLiteral("midi");
static auto const PERIOD            = QStringLiteral("period");
static auto const PIANO             = QStringLiteral("piano");
static auto const PORTNAME          = QStringLiteral("portname");
static auto const QUALITY           = QStringLiteral("quality");
static auto const SAMPLERATE_INDEX  = QStringLiteral("samplerateIndex");
static auto const SHOW_FLATS        = QStringLiteral("showFlats");
static auto const SHOW_PREVIEWS     = QStringLiteral("showPreviews");
static auto const SOUND             = QStringLiteral("sound");

}


//
// utility function that makes:
// assignSetting(mySetting, settings, key, defaultValue)
// equivalent to:
// mySetting = settings.value(key, defaultValue).value<decltype(mySetting)>();
//
template <class T>
static void assignSetting(T &lhs, QSettings &settings, QString const& key, QVariant const& defaultValue = QVariant()) {
    lhs = settings.value(key, defaultValue).value<T>();
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


    settings.beginGroup(Keys::CONFIG);

    settings.beginGroup(Keys::APPEARANCE);

    {
        QString fontfamily;
        int fontSize;
        assignSetting(fontfamily, settings, Keys::FONT_FAMILY, Appearance::DEFAULT_FONT_FAMILY);
        assignSetting(fontSize, settings, Keys::FONT_SIZE, Appearance::DEFAULT_FONT_SIZE);
        mAppearance.font.setFamily(fontfamily);
        mAppearance.font.setPointSize(fontSize);
    }

    settings.beginReadArray(Keys::COLORS);
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
   
    assignSetting(mAppearance.showFlats, settings, Keys::SHOW_FLATS, Appearance::DEFAULT_SHOW_FLATS);
    assignSetting(mAppearance.showPreviews, settings, Keys::SHOW_PREVIEWS, Appearance::DEFAULT_SHOW_PREVIEWS);

    settings.endGroup(); // appearance

    settings.beginGroup(Keys::GENERAL);
    assignSetting(mGeneral.historyLimit, settings, Keys::HISTORY_LIMIT, General::DEFAULT_HISTORY);
    if (mGeneral.historyLimit < General::MIN_HISTORY || mGeneral.historyLimit > General::MAX_HISTORY) {
        qWarning() << LOG_PREFIX << "invalid history setting, using default";
    }
    settings.endGroup(); // general

    settings.beginGroup(Keys::KEYBOARD);

    // keybindings for the piano widget + note column in pattern editor
    settings.beginReadArray(Keys::PIANO);
    for (int i = 0; i != sizeof(DEFAULT_PIANO_BINDINGS) / sizeof(Qt::Key); ++i) {
        settings.setArrayIndex(i);
        mKeyboard.pianoInput.bind(
            static_cast<Qt::Key>(settings.value(Keys::KEYCODE, DEFAULT_PIANO_BINDINGS[i]).toInt()),
            i
        );
    }
    settings.endArray();

    settings.endGroup(); // keyboard


    settings.beginGroup(Keys::MIDI);
    {
        assignSetting(mMidi.enabled, settings, Keys::ENABLED, Midi::DEFAULT_ENABLED);
        RtMidi::Api api;
        assignSetting((int&)api, settings, Keys::API, RtMidi::UNSPECIFIED);
        QString portName;
        assignSetting(portName, settings, Keys::PORTNAME);

    
        auto &prober = MidiProber::instance();
        if (api == RtMidi::UNSPECIFIED) {
            mMidi.backendIndex = 0; // default to first available
        } else {
            mMidi.backendIndex = prober.indexOfApi(api);
            if (mMidi.backendIndex == -1) {
                // this should rarely happen
                qWarning() << LOG_PREFIX << "MIDI API is not available, falling back to the first available";
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
                qWarning() << LOG_PREFIX << "Could not find MIDI port, please select a new device";
            }
        }
        
    }
    settings.endGroup(); // midi

    //
    // Sound configuration notes:
    // If the API cannot be found the first one available is chosen
    // If the device name cannot be found, the default one is chosen
    //

    settings.beginGroup(Keys::SOUND);
    {
        auto &audioProber = AudioProber::instance();
        // get the configured backend
        ma_backend backend;
        assignSetting((int&)backend, settings, Keys::BACKEND, MA_BACKEND_COUNT);
        if (backend == MA_BACKEND_COUNT) {
            mSound.backendIndex = 0;
        } else {
            mSound.backendIndex = audioProber.indexOfBackend(backend);
            if (mSound.backendIndex == -1) {
                auto backendName = ma_get_backend_name(backend);
                qWarning().noquote() << LOG_PREFIX << "audio API" << backendName << "not available";
                mSound.backendIndex = 0; // default to the first one
            }
        }

        audioProber.probe(mSound.backendIndex);
        
        // get the configured device name
        QByteArray idData;
        assignSetting(idData, settings, Keys::DEVICE_ID);
        if (idData.size() == sizeof(ma_device_id)) {
            int index = audioProber.indexOfDevice(
                mSound.backendIndex,
                *reinterpret_cast<ma_device_id*>(idData.data())
            );

            if (index == 0) {
                // device not available, use the default
                qWarning() << LOG_PREFIX << "last configured device not available, using default";
            }
            mSound.deviceIndex = index;
        } else {
            mSound.deviceIndex = 0;
        }

    }

    assignSetting(mSound.samplerateIndex, settings, Keys::SAMPLERATE_INDEX, Sound::DEFAULT_SAMPLERATE);
    if (mSound.samplerateIndex < 0 || mSound.samplerateIndex >= N_SAMPLERATES) {
        qWarning() << LOG_PREFIX << "invalid samplerate in config, loading default";
        mSound.samplerateIndex = Sound::DEFAULT_SAMPLERATE;
    }

    assignSetting(mSound.latency, settings, Keys::LATENCY, Sound::DEFAULT_LATENCY);
    if (mSound.latency < Sound::MIN_LATENCY || mSound.latency > Sound::MAX_LATENCY) {
        qWarning() << LOG_PREFIX << "invalid latency in config, loading default";
        mSound.latency = Sound::DEFAULT_LATENCY;
    }
    
    assignSetting(mSound.period, settings, Keys::PERIOD, Sound::DEFAULT_PERIOD);
    if (mSound.period < Sound::MIN_PERIOD || mSound.period > Sound::MAX_PERIOD) {
        qWarning() << LOG_PREFIX << "invalid period in config, loading default";
        mSound.period = Sound::DEFAULT_PERIOD;
    }

    assignSetting(mSound.quality, settings, Keys::QUALITY, Sound::DEFAULT_QUALITY);
    if (mSound.quality < (int)gbapu::Apu::Quality::low || mSound.quality > (int)gbapu::Apu::Quality::high) {
        qWarning() << LOG_PREFIX << "invalid quality in config, loading default";
        mSound.quality = Sound::DEFAULT_QUALITY;
    }

    settings.endGroup(); // sound

    settings.endGroup(); // config
}

void Config::readColor(QSettings &settings, Color color, QColor def) {
    settings.setArrayIndex(+color);
    assignSetting(mAppearance.colors[+color], settings, Keys::COLOR, def);
}


void Config::writeSettings() {
    QSettings settings;
    settings.beginGroup(Keys::CONFIG);

    settings.beginGroup(Keys::APPEARANCE);
    settings.setValue(Keys::FONT_FAMILY, mAppearance.font.family());
    settings.setValue(Keys::FONT_SIZE, mAppearance.font.pointSize());
    settings.beginWriteArray(Keys::COLORS, (int)mAppearance.colors.size());
    for (size_t i = 0; i != mAppearance.colors.size(); ++i) {
        settings.setArrayIndex((int)i);
        settings.setValue(Keys::COLOR, mAppearance.colors[i]);
    }
    settings.endArray();
    settings.setValue(Keys::SHOW_FLATS, mAppearance.showFlats);
    settings.setValue(Keys::SHOW_PREVIEWS, mAppearance.showPreviews);
    settings.endGroup();

    settings.beginGroup(Keys::GENERAL);
    settings.setValue(Keys::HISTORY_LIMIT, mGeneral.historyLimit);
    settings.endGroup();

    settings.beginGroup(Keys::KEYBOARD);
    settings.beginWriteArray(Keys::PIANO);
    auto &bindings = mKeyboard.pianoInput.bindings();
    for (size_t i = 0; i != bindings.size(); ++i) {
        settings.setArrayIndex((int)i);
        settings.setValue(Keys::KEYCODE, bindings[i]);
    }
    settings.endArray();
    settings.endGroup();

    settings.beginGroup(Keys::MIDI);
    {
        settings.setValue(Keys::ENABLED, mMidi.enabled);
        auto &prober = MidiProber::instance();
        settings.setValue(Keys::API, (int)prober.api());
        QString str;
        if (mMidi.portIndex != -1) {
            str = prober.portNames().at(mMidi.portIndex);
        }
        settings.setValue(Keys::PORTNAME, str);
    }
    settings.endGroup();

    settings.beginGroup(Keys::SOUND);

    {
        auto &audioProber = AudioProber::instance();
        auto context = audioProber.context(mSound.backendIndex);
        int backend = MA_BACKEND_COUNT;
        if (context) {
            backend = context->backend;
        }
        settings.setValue(Keys::BACKEND, backend);

        QByteArray id;
        auto devId = audioProber.deviceId(mSound.backendIndex, mSound.deviceIndex);
        if (devId) {
            id = QByteArray(reinterpret_cast<char*>(devId), sizeof(ma_device_id));
        }
        settings.setValue(Keys::DEVICE_ID, id);
    }

    settings.setValue(Keys::SAMPLERATE_INDEX, mSound.samplerateIndex);
    settings.setValue(Keys::LATENCY, mSound.latency);
    settings.setValue(Keys::PERIOD, mSound.period);
    settings.setValue(Keys::QUALITY, mSound.quality);
    settings.endGroup();

    settings.endGroup();
}

void Config::disableMidi() {
    mMidi.enabled = false;
}
