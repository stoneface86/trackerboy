
#pragma once

#include <memory>

#include <QMainWindow>
#include <QFileDialog>



#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"
#include "ConfigDialog.hpp"
#include "InstrumentEditor.hpp"
#include "Renderer.hpp"
#include "WaveEditor.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

protected:

    void closeEvent(QCloseEvent *evt) override;

private slots:
    void updateWindowTitle();

    // actions
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();

private:

    // To be called before loading a new document. Prompts user to save if the
    // current document is modified. Returns false if the user does not want to continue
    bool maybeSave();

    void readSettings();

    void setFilename(QString filename);

    void setModelsEnabled(bool enabled);

    void writeSettings();

    std::unique_ptr<Ui::MainWindow> mUi;

    QFileDialog *mModuleFileDialog;

    Config *mConfig;
    ModuleDocument *mDocument;
    InstrumentListModel *mInstrumentModel;
    WaveListModel *mWaveModel;
    

    InstrumentEditor *mInstrumentEditor;
    WaveEditor *mWaveEditor;
    ConfigDialog *mConfigDialog;

    Renderer *mRenderer;

    // file name of the currently open file or "Untitled" for a new file
    QString mFilename;
    QString mDocumentName;

};
