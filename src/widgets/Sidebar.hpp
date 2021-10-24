
#pragma once

#include "core/model/PatternModel.hpp"
#include "core/model/SongModel.hpp"
#include "core/model/SongListModel.hpp"
#include "widgets/sidebar/AudioScope.hpp"
#include "widgets/sidebar/OrderEditor.hpp"
#include "widgets/sidebar/SongEditor.hpp"

#include <QAction>
#include <QComboBox>
#include <QWidget>

//
// Composite widget for the tracker sidebar. This sits beside the pattern editor
// and contains the order editor, song chooser, song settings editor, and the visualizer.
//
class Sidebar : public QWidget {

    Q_OBJECT

public:

    explicit Sidebar(
        Module &mod,
        PatternModel &patternModel,
        SongListModel &songListModel,
        SongModel &songModel,
        QWidget *parent = nullptr
    );

    AudioScope* scope();

    OrderEditor* orderEditor();

    SongEditor* songEditor();

    QAction* nextSongAction();

    QAction* previousSongAction();

public slots:

    //
    // Selects the next song in the list
    //
    void nextSong();

    //
    // Selects the previous song in the list
    //
    void previousSong();

private slots:

    void reload();

private:

    Q_DISABLE_COPY(Sidebar)

    void updateActions();

    AudioScope *mScope;
    OrderEditor *mOrderEditor;
    SongEditor *mSongEditor;
    QComboBox *mSongChooser;

    QAction *mNextAction;
    QAction *mPrevAction;

};
