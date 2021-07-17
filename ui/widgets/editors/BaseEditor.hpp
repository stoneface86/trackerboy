
#pragma once

#include "core/Document.hpp"
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

    PianoWidget& piano();

public slots:
    
    void setDocument(Document *document);

    //
    // Opens the item at the given index for editing
    // 
    void openItem(int index);

protected:

    explicit BaseEditor(PianoInput const& input, QString typeName, QWidget *parent = nullptr);

    QWidget& editorWidget();


    virtual BaseTableModel* getModel(Document &doc) = 0;

    Document *document();

protected:
    virtual void setCurrentItem(int index) = 0;

    virtual void documentChanged(Document *doc);

private slots:

    void onIndexChanged(int index);

    void onNameEdited(QString const& name);

    void add();

    void remove();

    void duplicate();

private:

    Document *mDocument;

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
