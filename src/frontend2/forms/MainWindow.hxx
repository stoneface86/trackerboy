
#pragma once

#include <QMainWindow>
#include <QSpinBox>
#include <QToolBar>

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

    void updateIcons();

    // slots
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExportToWav();
    void onRecentFile();
    void onConfiguration();

    void showSongListEditor();

    void showModuleProperties();

    void showComments();

    void stopPlayback();

    void initMenuBar();
    void initStatusBar();
    void initUi();

    RecentFiles mRecentFiles;
    Document *mDocument;
    SongListModel *mSongListModel;
    SongListEditor *mSongListEditor;
    ModulePropertiesDialog *mModuleProperties;
    CommentsDialog *mComments;

    // toolbars
    QToolBar *mToolbarFile;
    QToolBar *mToolbarEdit;
    QToolBar *mToolbarSong;
    QToolBar *mToolbarTracker;
    QToolBar *mToolbarInput;
    QSpinBox *mOctaveSpin;

    // Central widget
    // QHBoxLayout mLayout;
    //   Sidebar mSidebar;
    //   QSplitter mHSplitter;
    //     PatternEditor mPatternEditor;
    //     QSplitter mVSplitter;
    //       TableView mInstruments;
    //       TableView mWaveforms;
};
