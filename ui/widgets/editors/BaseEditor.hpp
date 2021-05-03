
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "widgets/PianoWidget.hpp"

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QWidget>


class BaseEditor : public QWidget {

    Q_OBJECT

public:


public slots:
    void setDocument(ModuleDocument *document);

    //
    // Opens the item at the given index for editing
    // 
    void openItem(int index);

protected:

    explicit BaseEditor(PianoInput const& input, QString typeName, QWidget *parent = nullptr);

    QWidget& editorWidget();

    virtual BaseTableModel* getModel(ModuleDocument &doc) = 0;

    ModuleDocument *document();

protected:
    virtual void setCurrentItem(int index) = 0;

private slots:

    void onIndexChanged(int index);

    void onNameEdited(QString const& name);

    void add();

    void remove();

    void duplicate();

private:

    ModuleDocument *mDocument;

    bool mSignalsEnabled;

    QVBoxLayout mLayout;
        QHBoxLayout mTableLayout;
            QToolBar mToolbar;
            QComboBox mCombo;
            QLabel mNameLabel;
            QLineEdit mNameEdit;
        QWidget mEditorWidget;
        PianoWidget mPiano;

    // actions
    QAction mActionAdd;
    QAction mActionRemove;
    QAction mActionDuplicate;
    QAction mActionImport;
    QAction mActionExport;

};
