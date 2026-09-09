
#pragma once

class QCommandLineParser;
#include <QString>

//
// Command line front-end for WavExporter. Lets a module be exported to WAV
// without starting the editor, for example:
//
//   trackerboy song.tbm --export-wav song.wav
//   trackerboy song.tbm --export-wav out/ --separate --channels 1,2
//
// The GUI counterpart is ExportWavDialog.
//
namespace ExportWavCommand {

    enum class Status {
        success,        // export completed
        badArguments,   // usage error: missing or invalid arguments
        failed          // the module could not be loaded or the export failed
    };

    //
    // Returns true if the export option is present in the raw program
    // arguments. This is checked before a QApplication is constructed so that
    // exporting does not require a display.
    //
    bool requested(int argc, char *argv[]);

    //
    // Adds the export options to the parser.
    //
    void addOptions(QCommandLineParser &parser);

    //
    // Returns the name of the first option that only applies to exporting
    // and was set without the export option itself, or an empty string
    // if there is no such option.
    //
    QString strayOption(QCommandLineParser const& parser);

    //
    // Runs the export using the parsed options. modulePath is the module file
    // to export. Progress and errors are written to stderr.
    //
    Status run(QCommandLineParser const& parser, QString const& modulePath);

}
