
#pragma once

#include "core/clipboard/PatternClip.hpp"

#include <QObject>

#include <optional>

//
// Utility class for managing the system clipboard.
//
class PatternClipboard : public QObject {

    Q_OBJECT

public:

    explicit PatternClipboard(QObject *parent = nullptr);

    bool hasClip() const;

    PatternClip const& clip() const;

    //
    // Puts the given pattern clip on the system clipboard. This class takes
    // ownership of the clip.
    //
    void setClip(PatternClip &clip);

private:
    void parseClipboard();

private:
    std::optional<PatternClip> mClip;
    bool mSettingClipboard;



};
