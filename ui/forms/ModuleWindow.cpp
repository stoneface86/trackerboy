
#include "forms/ModuleWindow.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

const char* ModuleWindow::MODULE_FILE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");


ModuleWindow::ModuleWindow(Config &config, ModuleDocument *doc, QWidget *parent) :
    QWidget(parent),
    mConfig(config),
    mDocument(doc),
    mLayout(),
    mOrderWidget(),
    mPatternEditor(config.keyboard().pianoInput, *doc)
{

    mLayout.addWidget(&mOrderWidget);
    mLayout.addWidget(&mPatternEditor, 1);
    mLayout.setMargin(0);
    setLayout(&mLayout);




    updateWindowTitle();
    connect(doc, &ModuleDocument::modifiedChanged, this, &QWidget::setWindowModified);
}

void ModuleWindow::applyConfiguration(Config::Categories categories) {
    if (categories.testFlag(Config::CategoryAppearance)) {
        auto &appearance = mConfig.appearance();
        mPatternEditor.setColors(appearance.colors);
        mDocument->orderModel().setRowColor(appearance.colors[+Color::row]);
    }
}

ModuleDocument* ModuleWindow::document() noexcept {
    return mDocument;
}

OrderEditor& ModuleWindow::orderEditor() noexcept {
    return mOrderWidget;
}

PatternEditor& ModuleWindow::patternEditor() noexcept {
    return mPatternEditor;
}

bool ModuleWindow::save() {
    if (mDocument->hasFile()) {
        return mDocument->save();
    } else {
        return saveAs();
    }
}

bool ModuleWindow::saveAs() {

    auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        "",
        tr(MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    auto result = mDocument->save(path);
    if (result) {
        updateWindowTitle();
    }
    return result;
}

void ModuleWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        emit documentClosed(mDocument);
        evt->accept();
    } else {
        evt->ignore();
    }

    
}

void ModuleWindow::showEvent(QShowEvent *evt) {
    // focus on the pattern editor when the window is activated
    mPatternEditor.setFocus();
    evt->accept();
}

bool ModuleWindow::maybeSave() {

    if (!mDocument->isModified()) {
        return true;
    }

    auto const result = QMessageBox::warning(
        this,
        tr("Trackerboy"),
        tr("Save changes to %1?").arg(mDocument->name()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    switch (result) {
        case QMessageBox::Save:
            return save();
        case QMessageBox::Cancel:
            return false;
        default:
            break;
    }

    return true;
}

void ModuleWindow::updateWindowTitle() {
    setWindowTitle(QStringLiteral("%1[*]").arg(mDocument->name()));
}
