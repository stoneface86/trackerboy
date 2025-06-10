
#pragma once

#include <QMainWindow>

#include "core/Document.hxx"
#include "forms/CommentsDialog.hxx"
#include "forms/ModulePropertiesDialog.hxx"
#include "forms/SongListEditor.hxx"
#include "model/SongListModel.hxx"
#include "utils/RecentFiles.hxx"

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

    void openFile(QString const &path);

    void panic(QString const &msg);

private:
    Q_DISABLE_COPY(MainWindow)

    void showSongListEditor();

    void showModuleProperties();

    void showComments();

    void stopPlayback();

    void initMenuBar();

    RecentFiles mRecentFiles;
    Document *mDocument;
    SongListModel *mSongListModel;
    SongListEditor *mSongListEditor;
    ModulePropertiesDialog *mModuleProperties;
    CommentsDialog *mComments;

    // Central widget
    // QHBoxLayout mLayout;
    //   Sidebar mSidebar;
    //   QSplitter mHSplitter;
    //     PatternEditor mPatternEditor;
    //     QSplitter mVSplitter;
    //       TableView mInstruments;
    //       TableView mWaveforms;
};
