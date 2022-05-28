
#pragma once

#include "model/BaseTableModel.hpp"
#include "core/Module.hpp"
#include "forms/PersistantDialog.hpp"
#include "widgets/PianoWidget.hpp"
#include "verdigris/wobjectdefs.h"

#include <QComboBox>
#include <QLineEdit>


class BaseEditor : public PersistantDialog {

    W_OBJECT(BaseEditor)

public:

    PianoWidget* piano();
    
    int currentItem() const;

    // needed because we can't call virtual functions in BaseEditor's constructor
    void init();

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

    virtual void setCurrentItem(int index) = 0;
    
    BaseTableModel &mModel;

private:

    void onIndexChanged(int index);

    void onNameEdited(QString const& name);

    QComboBox *mCombo;
    QLineEdit *mNameEdit;
    QWidget *mEditorWidget;
    PianoWidget *mPiano;

};
