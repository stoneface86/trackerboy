
#pragma once

#include <QMainWindow>
#include <QSplitter>

#include <array>

#include "core/Document.hxx"
#include "forms/CommentsDialog.hxx"
#include "forms/ModulePropertiesDialog.hxx"
#include "forms/SongListEditor.hxx"
#include "model/NameListModel.hxx"
#include "utils/RecentFiles.hxx"
#include "widgets/DataWidget.hxx"
#include "widgets/Sidebar.hxx"

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow();

    void openFile(QString const &path);

    void panic(QString const &msg);

protected:
    void closeEvent(QCloseEvent *evt) override;

private:
    Q_DISABLE_COPY(MainWindow)

    void updateIcons();
    void updateSongSelectActions();

    // slots
    void onNew();
    void onOpen();
    [[nodiscard]] bool onSave();
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

    void visitFile(QString const &path);
    [[nodiscard]] bool save(QString const &dest, bool updateName);
    [[nodiscard]] bool saveAs(QString const &startingPath);
    [[nodiscard]] bool canReload();

    void setModulePath();
    void setModulePath(QString const &path);
    QString modulePath() const;

    void statusMessage(QString const &message);

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

    QString _lastFileDir;
    bool _isUntitled;
    bool _autoBackup;

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
