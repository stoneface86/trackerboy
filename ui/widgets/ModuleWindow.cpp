
#include "widgets/ModuleWindow.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

static auto MODULE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");


ModuleWindow::ModuleWindow(ModuleDocument *doc, QWidget *parent) :
    QWidget(parent),
    mLayout(),
    mTabs(),
    mTabSettings(*doc),
    mTabPatterns(*doc),
    mTabInstruments(*doc),
    mTabWaveforms(*doc),
    mDocument(doc)
{

    mTabs.addTab(&mTabSettings, tr("General settings"));
    mTabs.addTab(&mTabPatterns, tr("Patterns"));
    mTabs.addTab(&mTabInstruments, tr("Instruments"));
    mTabs.addTab(&mTabWaveforms, tr("Waveforms"));


    mLayout.setMargin(0);
    mLayout.addWidget(&mTabs);
    setLayout(&mLayout);




    updateWindowTitle();
    connect(doc, &ModuleDocument::modifiedChanged, this, &QWidget::setWindowModified);
}

ModuleDocument* ModuleWindow::document() noexcept {
    return mDocument;
}

ModuleDocument* ModuleWindow::open(QWidget *parent) {
    auto path = QFileDialog::getOpenFileName(
        parent,
        tr("Open module"),
        "",
        tr(MODULE_FILTER)
    );

    if (path.isEmpty()) {
        return nullptr;
    }
    auto *doc = new ModuleDocument(path, parent);
    return doc;

}

bool ModuleWindow::save() {
    if (mDocument->hasFile()) {
        commit();
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
        tr(MODULE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    commit();
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

void ModuleWindow::commit() {
    mTabSettings.commit();
}

void ModuleWindow::updateWindowTitle() {
    setWindowTitle(QStringLiteral("%1[*]").arg(mDocument->name()));
}
