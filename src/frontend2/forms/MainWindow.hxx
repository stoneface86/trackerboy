
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
#include "model/SongListModel.hxx"
#include "utils/RecentFiles.hxx"
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

    void initToolBars();
    void initMenuBar();
    void initStatusBar();
    void initUi();
    void loadSettings();
    void saveSettings();

    RecentFiles mRecentFiles;
    Document *mDocument;
    SongListModel *mSongListModel;
    SongModel *mSongModel;
    SongListEditor *mSongListEditor;
    ModulePropertiesDialog *mModuleProperties;
    CommentsDialog *mComments;

    // toolbars
    enum Toolbars {
        ToolbarFile,
        ToolbarEdit,
        ToolbarTracker,
        ToolbarInput,
        ToolbarView,
        ToolbarCount
    };
    std::array<QToolBar *, ToolbarCount> mToolbars;

    struct Ui {
        // QLabel is a placeholder
        Sidebar *sidebar;
        QSplitter *hsplitter;
        QLabel *editor;
        QSplitter *databar;
        QLabel *instruments;
        QLabel *waveforms;
        // toolbar widgets
        QSpinBox *inputOctaveSpin;
        QSpinBox *inputEditStep;
        // actions that we need to reference later
        QAction *actKeyRepeat;
        QAction *actFollowMode;
        QAction *actPatternRepeat;
        QAction *actRecord;
        QAction *actShowSidebar;
        QAction *actShowDatabar;
        QAction *actShowStatusbar;
    };

    Ui mUi;
};
