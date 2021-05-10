
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "core/Trackerboy.hpp"
#include "core/Config.hpp"

#include "widgets/module/OrderEditor.hpp"
#include "widgets/PatternEditor.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QWidget>

class ModuleWindow : public QWidget {

    Q_OBJECT

public:

    explicit ModuleWindow(Config &config, ModuleDocument *doc, QWidget *parent = nullptr);

    ModuleDocument* document() noexcept;

    OrderEditor& orderEditor() noexcept;

    PatternEditor& patternEditor() noexcept;

    bool save();

    bool saveAs();

    static const char* MODULE_FILE_FILTER;

public slots:
    void applyConfiguration(Config::Categories categories);

protected:

    virtual void closeEvent(QCloseEvent *evt) override;

    virtual void showEvent(QShowEvent *evt) override;

signals:
    void documentClosed(ModuleDocument *document);

private:

    void updateWindowTitle();

    bool maybeSave();

    Config &mConfig;
    ModuleDocument *mDocument;

    QHBoxLayout mLayout;
        OrderEditor mOrderWidget;
        PatternEditor mPatternEditor;




};
