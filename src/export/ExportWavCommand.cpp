
#include "export/ExportWavCommand.hpp"

#include "core/ChannelOutput.hpp"
#include "core/Module.hpp"
#include "core/ModuleFile.hpp"
#include "export/WavExporter.hpp"

#include <QByteArrayView>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringView>

#include <array>
#include <chrono>
#include <cstdio>
#include <optional>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#define TU ExportWavCommandTU
namespace TU {

#define cmd_tr(str) QCoreApplication::translate("ExportWavCommand", str)

// option names
constexpr auto OPT_EXPORT     = "export-wav";
constexpr auto OPT_CHANNELS   = "channels";
constexpr auto OPT_SEPARATE   = "separate";
constexpr auto OPT_PREFIX     = "prefix";
constexpr auto OPT_LOOPS      = "loops";
constexpr auto OPT_DURATION   = "duration";
constexpr auto OPT_SAMPLERATE = "samplerate";
constexpr auto OPT_SONG       = "song";

// options that only make sense together with OPT_EXPORT
constexpr std::array MODIFIERS = {
    OPT_CHANNELS,
    OPT_SEPARATE,
    OPT_PREFIX,
    OPT_LOOPS,
    OPT_DURATION,
    OPT_SAMPLERATE,
    OPT_SONG
};

constexpr int DEFAULT_SAMPLERATE = 44100;

void error(QString const& msg) {
    fputs(qPrintable(cmd_tr("error: %1\n").arg(msg)), stderr);
}

void info(QString const& msg) {
    fputs(qPrintable(msg), stderr);
    fputc('\n', stderr);
}

//
// Parses a channel list, ie "124" or "1,2,4". Commas and whitespace are
// ignored. nullopt is returned for invalid input or if no channels were given.
//
std::optional<ChannelOutput::Flags> parseChannels(QString const& str) {
    ChannelOutput::Flags flags = ChannelOutput::AllOff;
    for (auto const ch : str) {
        if (ch >= QChar('1') && ch <= QChar('4')) {
            flags |= (ChannelOutput::Flag)(1 << (ch.unicode() - '1'));
        } else if (ch != QChar(',') && !ch.isSpace()) {
            return std::nullopt;
        }
    }

    if (flags == ChannelOutput::AllOff) {
        return std::nullopt;
    }
    return flags;
}

//
// Parses a duration given in seconds ("90") or as minutes and seconds
// ("01:30"). Zero and invalid input result in nullopt.
//
std::optional<std::chrono::seconds> parseDuration(QString const& str) {
    bool ok = false;
    unsigned seconds = 0;

    auto const colon = str.indexOf(QChar(':'));
    if (colon == -1) {
        seconds = str.toUInt(&ok);
    } else {
        QStringView const view(str);
        auto const mins = view.first(colon).toUInt(&ok);
        if (ok) {
            auto const secs = view.sliced(colon + 1).toUInt(&ok);
            if (ok && secs < 60) {
                seconds = mins * 60 + secs;
            } else {
                ok = false;
            }
        }
    }

    if (!ok || seconds == 0) {
        return std::nullopt;
    }
    return std::chrono::seconds(seconds);
}

std::optional<int> parsePositiveInt(QString const& str) {
    bool ok;
    int const value = str.toInt(&ok);
    if (!ok || value <= 0) {
        return std::nullopt;
    }
    return value;
}

QString invalidValue(char const* option, QString const& value) {
    return cmd_tr("invalid value \"%1\" for --%2").arg(value, QLatin1String(option));
}

QString openError(ModuleFile const& file) {
    switch (file.lastError()) {
        case trackerboy::FormatError::invalidSignature:
            return cmd_tr("the file is not a trackerboy module");
        case trackerboy::FormatError::invalidRevision:
            return cmd_tr("the module is from a newer version of Trackerboy");
        case trackerboy::FormatError::cannotUpgrade:
            return cmd_tr("failed to upgrade the module");
        case trackerboy::FormatError::duplicateId:
        case trackerboy::FormatError::invalid:
        case trackerboy::FormatError::unknownChannel:
            return cmd_tr("the module is corrupted");
        default:
            return cmd_tr("the file could not be read");
    }
}

//
// Prints export progress to stderr. When stderr is a terminal the percentage
// of the current file is updated in place, otherwise each file is listed once.
//
class ProgressPrinter {

public:
    ProgressPrinter() :
        mIsTerminal(isatty(fileno(stderr))),
        mCurrent(),
        mMax(1),
        mLastPercent(-1)
    {
    }

    void fileStarted(QString const& filename) {
        finishCurrent();
        mCurrent = filename;
        mMax = 1;
        mLastPercent = -1;
        if (mIsTerminal) {
            printPercent(0);
        } else {
            info(QStringLiteral("  %1").arg(filename));
        }
    }

    void progressMax(int max) {
        mMax = qMax(1, max);
    }

    void progress(int amount) {
        if (mIsTerminal) {
            auto const percent = qBound(0, amount * 100 / mMax, 100);
            if (percent != mLastPercent) {
                printPercent(percent);
            }
        }
    }

    //
    // Marks the current file as complete
    //
    void finishCurrent() {
        if (mIsTerminal && !mCurrent.isEmpty()) {
            printPercent(100);
            fputc('\n', stderr);
            mCurrent.clear();
        }
    }

    //
    // Ends the in-place progress line, if any, without marking it complete
    //
    void abandonCurrent() {
        if (mIsTerminal && !mCurrent.isEmpty()) {
            fputc('\n', stderr);
            mCurrent.clear();
        }
    }

    QString const& current() const {
        return mCurrent;
    }

private:
    void printPercent(int percent) {
        mLastPercent = percent;
        fprintf(stderr, "\r  %s  %3d%%", qPrintable(mCurrent), percent);
        fflush(stderr);
    }

    bool mIsTerminal;
    QString mCurrent;
    int mMax;
    int mLastPercent;
};

}

namespace ExportWavCommand {

bool requested(int argc, char *argv[]) {
    QByteArrayView const optionName(TU::OPT_EXPORT);

    for (int i = 1; i < argc; ++i) {
        QByteArrayView arg(argv[i]);
        if (arg == QByteArrayView("--")) {
            // everything after "--" is a positional argument
            break;
        }
        if (!arg.startsWith('-')) {
            continue;
        }

        // QCommandLineParser accepts long options with one or two dashes,
        // with the value either separate or attached via '='
        arg = arg.sliced(arg.startsWith(QByteArrayView("--")) ? 2 : 1);
        auto const equals = arg.indexOf('=');
        if (equals != -1) {
            arg = arg.first(equals);
        }

        if (arg == optionName) {
            return true;
        }
    }

    return false;
}

void addOptions(QCommandLineParser &parser) {
    parser.addOptions({
        {
            QLatin1String(TU::OPT_EXPORT),
            cmd_tr("Export the module to WAV and exit without starting the editor. "
                   "<output> is the file to write, or the directory to write to "
                   "when --separate is given."),
            cmd_tr("output")
        },
        {
            QLatin1String(TU::OPT_CHANNELS),
            cmd_tr("Channels to export, as digits 1-4 (ie \"124\" or \"1,2,4\"). "
                   "Default: all channels."),
            cmd_tr("channels")
        },
        {
            QLatin1String(TU::OPT_SEPARATE),
            cmd_tr("Export each channel to its own file, named <prefix>.chN.wav, "
                   "in the <output> directory.")
        },
        {
            QLatin1String(TU::OPT_PREFIX),
            cmd_tr("Filename prefix for --separate. Default: the module "
                   "filename without its extension."),
            cmd_tr("prefix")
        },
        {
            QLatin1String(TU::OPT_LOOPS),
            cmd_tr("Play the song <count> times. Default: 1. "
                   "Cannot be combined with --duration."),
            cmd_tr("count")
        },
        {
            QLatin1String(TU::OPT_DURATION),
            cmd_tr("Play for a fixed time, in seconds or mm:ss, instead of looping."),
            cmd_tr("time")
        },
        {
            QLatin1String(TU::OPT_SAMPLERATE),
            cmd_tr("Samplerate of the exported audio, in Hz. Default: %1.")
                .arg(TU::DEFAULT_SAMPLERATE),
            cmd_tr("hz")
        },
        {
            QLatin1String(TU::OPT_SONG),
            cmd_tr("The song in the module to export, starting at 1. Default: 1."),
            cmd_tr("number")
        }
    });
}

QString strayOption(QCommandLineParser const& parser) {
    if (parser.isSet(QLatin1String(TU::OPT_EXPORT))) {
        return {};
    }

    for (auto const option : TU::MODIFIERS) {
        if (parser.isSet(QLatin1String(option))) {
            return QLatin1String(option);
        }
    }

    return {};
}

Status run(QCommandLineParser const& parser, QString const& modulePath) {

    // Arguments ------------------------------------------------------------

    if (modulePath.isEmpty()) {
        TU::error(cmd_tr("a module file is required when exporting"));
        return Status::badArguments;
    }

    auto const output = parser.value(QLatin1String(TU::OPT_EXPORT));

    ChannelOutput::Flags channels = ChannelOutput::AllOn;
    if (parser.isSet(QLatin1String(TU::OPT_CHANNELS))) {
        auto const value = parser.value(QLatin1String(TU::OPT_CHANNELS));
        auto const parsed = TU::parseChannels(value);
        if (!parsed) {
            TU::error(TU::invalidValue(TU::OPT_CHANNELS, value));
            return Status::badArguments;
        }
        channels = *parsed;
    }

    trackerboy::Player::Duration duration = 1;
    bool const hasLoops = parser.isSet(QLatin1String(TU::OPT_LOOPS));
    bool const hasDuration = parser.isSet(QLatin1String(TU::OPT_DURATION));
    if (hasLoops && hasDuration) {
        TU::error(cmd_tr("--%1 and --%2 cannot be used together")
                    .arg(QLatin1String(TU::OPT_LOOPS), QLatin1String(TU::OPT_DURATION)));
        return Status::badArguments;
    } else if (hasLoops) {
        auto const value = parser.value(QLatin1String(TU::OPT_LOOPS));
        auto const parsed = TU::parsePositiveInt(value);
        if (!parsed) {
            TU::error(TU::invalidValue(TU::OPT_LOOPS, value));
            return Status::badArguments;
        }
        duration = *parsed;
    } else if (hasDuration) {
        auto const value = parser.value(QLatin1String(TU::OPT_DURATION));
        auto const parsed = TU::parseDuration(value);
        if (!parsed) {
            TU::error(TU::invalidValue(TU::OPT_DURATION, value));
            return Status::badArguments;
        }
        duration = *parsed;
    }

    int samplerate = TU::DEFAULT_SAMPLERATE;
    if (parser.isSet(QLatin1String(TU::OPT_SAMPLERATE))) {
        auto const value = parser.value(QLatin1String(TU::OPT_SAMPLERATE));
        auto const parsed = TU::parsePositiveInt(value);
        if (!parsed) {
            TU::error(TU::invalidValue(TU::OPT_SAMPLERATE, value));
            return Status::badArguments;
        }
        samplerate = *parsed;
    }

    int songNumber = 1;
    if (parser.isSet(QLatin1String(TU::OPT_SONG))) {
        auto const value = parser.value(QLatin1String(TU::OPT_SONG));
        auto const parsed = TU::parsePositiveInt(value);
        if (!parsed) {
            TU::error(TU::invalidValue(TU::OPT_SONG, value));
            return Status::badArguments;
        }
        songNumber = *parsed;
    }

    bool const separate = parser.isSet(QLatin1String(TU::OPT_SEPARATE));
    bool const hasPrefix = parser.isSet(QLatin1String(TU::OPT_PREFIX));
    QString prefix;
    if (separate) {
        if (hasPrefix) {
            prefix = parser.value(QLatin1String(TU::OPT_PREFIX));
        } else {
            prefix = QFileInfo(modulePath).completeBaseName();
        }
        if (prefix.isEmpty()) {
            TU::error(cmd_tr("--%1 cannot be empty").arg(QLatin1String(TU::OPT_PREFIX)));
            return Status::badArguments;
        }
    } else if (hasPrefix) {
        TU::error(cmd_tr("--%1 requires --%2")
                    .arg(QLatin1String(TU::OPT_PREFIX), QLatin1String(TU::OPT_SEPARATE)));
        return Status::badArguments;
    }

    // Module ---------------------------------------------------------------

    QFileInfo const moduleInfo(modulePath);
    if (!moduleInfo.exists()) {
        TU::error(cmd_tr("module file \"%1\" does not exist").arg(modulePath));
        return Status::failed;
    }
    if (!moduleInfo.isFile()) {
        TU::error(cmd_tr("\"%1\" is not a file").arg(modulePath));
        return Status::failed;
    }

    Module mod;
    ModuleFile moduleFile;
    if (!moduleFile.open(modulePath, mod)) {
        TU::error(cmd_tr("could not open module: %1").arg(TU::openError(moduleFile)));
        return Status::failed;
    }

    auto const songCount = mod.data().songs().size();
    if (songNumber > songCount) {
        TU::error(cmd_tr("cannot export song %1, the module only has %2 song(s)")
                    .arg(songNumber)
                    .arg(songCount));
        return Status::badArguments;
    }
    mod.setSong(songNumber - 1);

    // Destination ----------------------------------------------------------

    if (separate) {
        if (!QDir(output).mkpath(QStringLiteral("."))) {
            TU::error(cmd_tr("could not create output directory \"%1\"").arg(output));
            return Status::failed;
        }
    } else {
        QFileInfo const outputInfo(output);
        if (outputInfo.isDir()) {
            TU::error(cmd_tr("\"%1\" is a directory (use --%2 to export files into a directory)")
                        .arg(output, QLatin1String(TU::OPT_SEPARATE)));
            return Status::badArguments;
        }
        if (!outputInfo.dir().exists()) {
            TU::error(cmd_tr("output directory \"%1\" does not exist").arg(outputInfo.dir().path()));
            return Status::failed;
        }
    }

    // Export ---------------------------------------------------------------

    TU::info(cmd_tr("Exporting %1, song %2# %3 (%4 Hz)")
                .arg(moduleInfo.fileName())
                .arg(songNumber)
                .arg(QString::fromStdString(mod.song()->name()))
                .arg(samplerate));

    WavExporter exporter(mod, samplerate);
    exporter.setDuration(duration);
    exporter.setChannels(channels);
    exporter.setSeparate(separate);
    exporter.setDestination(output);
    exporter.setSeparatePrefix(prefix);

    // the exporter runs in its own thread and this thread blocks until it
    // finishes, so these connections are direct: the slots are invoked from
    // the exporter's thread, which is fine since they only write to stderr.
    TU::ProgressPrinter printer;
    QObject::connect(&exporter, &WavExporter::fileStarted,
        [&printer](QString const& filename) { printer.fileStarted(filename); });
    QObject::connect(&exporter, &WavExporter::progressMax,
        [&printer](int max) { printer.progressMax(max); });
    QObject::connect(&exporter, &WavExporter::progress,
        [&printer](int amount) { printer.progress(amount); });

    exporter.start();
    exporter.wait();

    if (exporter.failed()) {
        auto const failedFile = printer.current().isEmpty() ? output : printer.current();
        printer.abandonCurrent();
        TU::error(cmd_tr("could not write \"%1\"").arg(failedFile));
        return Status::failed;
    }

    printer.finishCurrent();
    return Status::success;
}

}

#undef cmd_tr
#undef TU
