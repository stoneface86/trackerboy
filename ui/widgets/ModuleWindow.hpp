
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "widgets/module/InstrumentsWidget.hpp"
#include "widgets/module/ModuleSettingsWidget.hpp"
#include "widgets/module/PatternsWidget.hpp"
#include "widgets/module/WaveformsWidget.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QWidget>

class ModuleWindow : public QWidget {

    Q_OBJECT

public:

    explicit ModuleWindow(ModuleDocument *doc, QWidget *parent = nullptr);

    ModuleDocument* document() noexcept;

    bool save();

    bool saveAs();

    static ModuleDocument* open(QWidget *parent);

protected:

    virtual void closeEvent(QCloseEvent *evt) override;

signals:
    void documentClosed(ModuleDocument *document);

protected:
    ModuleDocument *mDocument;

private:

    void updateWindowTitle();

    bool maybeSave();

    //
    // Writes any retained data within the UI to the module
    //
    void commit();


    QGridLayout mLayout;
    QTabWidget mTabs;
        ModuleSettingsWidget mTabSettings;
        PatternsWidget mTabPatterns;
        InstrumentsWidget mTabInstruments;
        WaveformsWidget mTabWaveforms;




};
