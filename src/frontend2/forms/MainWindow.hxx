
#include <QMainWindow>

#include "core/Document.hxx"
#include "utils/RecentFiles.hxx"

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

    void openFile(QString const& path);

    void panic(QString const& msg);

private:
    Q_DISABLE_COPY(MainWindow)

    RecentFiles mRecentFiles;
    Document *mDocument;

    // Central widget
    // QHBoxLayout mLayout;
    //   Sidebar mSidebar;
    //   QSplitter mHSplitter;
    //     PatternEditor mPatternEditor;
    //     QSplitter mVSplitter;
    //       TableView mInstruments;
    //       TableView mWaveforms;

};
