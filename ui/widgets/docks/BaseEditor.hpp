
#pragma once

#include "core/model/BaseTableModel.hpp"
#include "core/Module.hpp"
#include "widgets/PianoWidget.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QWidget>


class BaseEditor : public QWidget {

    Q_OBJECT

public:

    PianoWidget* piano();

public slots:

    //
    // Opens the item at the given index for editing
    // 
    void openItem(int index);

protected:

    explicit BaseEditor(
        BaseTableModel &model,
        PianoInput const& input,
        QWidget *parent = nullptr
    );

    QWidget* editorWidget();



protected:
    int currentItem() const;

    virtual void setCurrentItem(int index) = 0;
    
    BaseTableModel &mModel;

private slots:

    void onIndexChanged(int index);

    void onNameEdited(QString const& name);

private:

    QComboBox *mCombo;
    QLineEdit *mNameEdit;
    QWidget *mEditorWidget;
    PianoWidget *mPiano;

};
