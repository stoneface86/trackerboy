
#pragma once

#include <QMainWindow>
#include <QSpinBox>
#include <QSplitter>
#include <QToolBar>

#include <array>

#include "core/Document.hxx"
#include "forms/CommentsDialog.hxx"
#include "forms/ModulePropertiesDialog.hxx"
#include "forms/SongListEditor.hxx"
#include "model/NameListModel.hxx"
#include "utils/RecentFiles.hxx"
#include "widgets/DataWidget.hxx"
#include "widgets/Sidebar.hxx"

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

    void openFile(QString const &path);

    void panic(QString const &msg);

protected:
    virtual void closeEvent(QCloseEvent *evt) override;

private:
    Q_DISABLE_COPY(MainWindow)

    void updateIcons();
    void updateSongSelectActions();

    // slots
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExportToWav();
    void onRecentFile();
    void onConfiguration();
    void onModulePrevSong();
    void onModuleNextSong();

    void showSongListEditor();

    void showModuleProperties();

    void showComments();

    void stopPlayback();

    void initToolBars();
    void initMenuBar();
    void initStatusBar();
    void initUi();
    void loadSettings();
    void saveSettings();

    RecentFiles _recentFiles;
    Document *_document;
    NameListModel *_songListModel;
    NameListModel *_instrumentListModel;
    NameListModel *_waveformListModel;
    SongModel *_songModel;
    SongListEditor *_songListEditor;
    ModulePropertiesDialog *_moduleProperties;
    CommentsDialog *_comments;

    // toolbars
    enum Toolbars {
        ToolbarFile,
        ToolbarEdit,
        ToolbarTracker,
        ToolbarInput,
        ToolbarView,
        ToolbarCount
    };
    std::array<QToolBar *, ToolbarCount> _toolbars;

    struct Ui {
        // QLabel is a placeholder
        Sidebar *sidebar;
        QSplitter *hsplitter;
        QLabel *editor;
        QSplitter *databar;
        DataWidget *instruments;
        DataWidget *waveforms;
        // toolbar widgets
        QSpinBox *inputOctaveSpin;
        QSpinBox *inputEditStep;
        // actions that we need to reference later
        QAction *actSongPrev;
        QAction *actSongNext;
        QAction *actKeyRepeat;
        QAction *actFollowMode;
        QAction *actPatternRepeat;
        QAction *actRecord;
        QAction *actShowSidebar;
        QAction *actShowDatabar;
        QAction *actShowStatusbar;
    };

    Ui _ui;
};
