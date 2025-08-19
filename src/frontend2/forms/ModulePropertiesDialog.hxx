
#pragma once

#include "core/Document.hxx"
#include "widgets/TickrateForm.hxx"

#include <QButtonGroup>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ModulePropertiesDialog : public QDialog {

    Q_OBJECT

public:
    explicit ModulePropertiesDialog(QWidget *parent = nullptr);

    //
    // Load data from the document to populate the dialog's forms
    //
    void load(Document const &doc);

    //
    // Applies changes to the document
    //
    void save(Document &doc) const;

private:
    Q_DISABLE_COPY(ModulePropertiesDialog)

    void setDirty();

    QLineEdit *_lineTitle;
    QLineEdit *_lineArtist;
    QLineEdit *_lineCopyright;

    TickrateForm *_tickrate;

    QLabel *_revisionLabel;
    QPushButton *_saveButton;
};