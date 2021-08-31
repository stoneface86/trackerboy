
#pragma once

#include "widgets/sidebar/AudioScope.hpp"
#include "widgets/sidebar/OrderEditor.hpp"
#include "widgets/sidebar/SongEditor.hpp"

#include <QWidget>

//
// Composite widget for the tracker sidebar. This sits beside the pattern editor
// and contains the order editor, song settings editor, and the visualizer.
//
class Sidebar : public QWidget {

    Q_OBJECT

public:

    explicit Sidebar(PatternModel &patternModel, SongModel &songModel, QWidget *parent = nullptr);

    AudioScope* scope();

    OrderEditor* orderEditor();

    SongEditor* songEditor();

private:

    AudioScope *mScope;
    OrderEditor *mOrderEditor;
    SongEditor *mSongEditor;

};
