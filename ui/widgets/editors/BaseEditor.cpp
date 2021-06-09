
#include "widgets/editors/BaseEditor.hpp"
#include "misc/IconManager.hpp"

#include <QtDebug>

BaseEditor::BaseEditor(PianoInput const& input, QString typeName, QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mSignalsEnabled(true),
    mLayout(),
    mTableLayout(),
    mToolbar(),
    mCombo(),
    mNameLabel(tr("Name:")),
    mNameEdit(),
    mEditorWidget(),
    mPiano(input)
{

    // layout
    mTableLayout.addWidget(&mToolbar);
    mTableLayout.addWidget(&mCombo, 1);
    mTableLayout.addWidget(&mNameLabel);
    mTableLayout.addWidget(&mNameEdit, 1);

    mLayout.addLayout(&mTableLayout);
    mLayout.addWidget(&mEditorWidget, 1);
    mLayout.addWidget(&mPiano);
    setLayout(&mLayout);

    mNameEdit.setEnabled(false);

    // actions
    mActionAdd.setText(tr("Add"));
    mActionAdd.setIcon(IconManager::getIcon(Icons::itemAdd));
    mActionAdd.setStatusTip(tr("Adds a new %1").arg(typeName));
    mActionRemove.setText(tr("Remove"));
    mActionRemove.setIcon(IconManager::getIcon(Icons::itemRemove));
    mActionRemove.setStatusTip(tr("Removes the current %1").arg(typeName));
    mActionDuplicate.setText(tr("Duplicate"));
    mActionDuplicate.setIcon(IconManager::getIcon(Icons::itemDuplicate));
    mActionDuplicate.setStatusTip(tr("Adds a copy of the current %1").arg(typeName));
    mActionImport.setText(tr("Import"));
    mActionImport.setIcon(IconManager::getIcon(Icons::itemImport));
    mActionImport.setStatusTip(tr("Import %1 from a file").arg(typeName));
    mActionExport.setText(tr("Export"));
    mActionExport.setIcon(IconManager::getIcon(Icons::itemExport));
    mActionExport.setStatusTip(tr("Export %1 to a file").arg(typeName));

    mToolbar.addAction(&mActionAdd);
    mToolbar.addAction(&mActionRemove);
    mToolbar.addAction(&mActionDuplicate);
    mToolbar.addAction(&mActionImport);
    mToolbar.addAction(&mActionExport);
    mToolbar.setIconSize(QSize(16, 16));

    mActionRemove.setEnabled(false);
    mActionDuplicate.setEnabled(false);
    // disable these until we add support for them
    mActionImport.setEnabled(false);
    mActionExport.setEnabled(false);

    connect(&mCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &BaseEditor::onIndexChanged);
    connect(&mNameEdit, &QLineEdit::textEdited, this, &BaseEditor::onNameEdited);
    connect(&mActionAdd, &QAction::triggered, this, &BaseEditor::add);
    connect(&mActionRemove, &QAction::triggered, this, &BaseEditor::remove);
    connect(&mActionDuplicate, &QAction::triggered, this, &BaseEditor::duplicate);

    setEnabled(false);
}

PianoWidget& BaseEditor::piano() {
    return mPiano;
}

void BaseEditor::setDocument(ModuleDocument *document) {
    mSignalsEnabled = false;

    mDocument = document;

    bool const hasDocument = document != nullptr;
    int index;

    if (hasDocument) {
        auto model = getModel(*document);
        mCombo.setModel(model);
        index = model->currentIndex();
        mCombo.setCurrentIndex(index);
    } else {
        mCombo.clear();
        mNameEdit.clear();
        index = -1;
    }

    setEnabled(hasDocument);

    mSignalsEnabled = true;
    onIndexChanged(index);
}

void BaseEditor::openItem(int index) {
    mCombo.setCurrentIndex(index);
}

QWidget& BaseEditor::editorWidget() {
    return mEditorWidget;
}

ModuleDocument* BaseEditor::document() {
    return mDocument;
}

void BaseEditor::onIndexChanged(int index) {
    if (mSignalsEnabled) {
        if (mDocument) {
            auto model = getModel(*mDocument);
            model->setCurrentIndex(index);

            bool const hasIndex = index != -1;
            if (hasIndex) {
                mNameEdit.setText(model->name(index));
            } else {
                mNameEdit.clear();
            }
            mNameEdit.setEnabled(hasIndex);
            mPiano.setEnabled(hasIndex);
            mEditorWidget.setEnabled(hasIndex);
            mActionRemove.setEnabled(hasIndex);
            mActionDuplicate.setEnabled(model->canDuplicate() && hasIndex);
        }

        setCurrentItem(index);
    }
}

void BaseEditor::onNameEdited(QString const& name) {
    auto model = getModel(*mDocument);
    model->rename(mCombo.currentIndex(), name);
}

void BaseEditor::add() {
    auto model = getModel(*mDocument);
    int index = model->add();
    mCombo.setCurrentIndex(index);
    if (!model->canDuplicate()) {
        mActionAdd.setEnabled(false);
        mActionDuplicate.setEnabled(false);
    }
}

void BaseEditor::remove() {
    auto model = getModel(*mDocument);
    model->remove(mCombo.currentIndex());
    mActionAdd.setEnabled(true);
}

void BaseEditor::duplicate() {
    auto model = getModel(*mDocument);
    int index = model->duplicate(mCombo.currentIndex());
    mCombo.setCurrentIndex(index);
    if (!model->canDuplicate()) {
        mActionAdd.setEnabled(false);
        mActionDuplicate.setEnabled(false);
    }
}

