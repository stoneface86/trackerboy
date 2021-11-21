
#pragma once

#include "model/BaseTableModel.hpp"
#include "core/Module.hpp"
#include "widgets/PianoWidget.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QDialog>


class BaseEditor : public QDialog {

    Q_OBJECT

public:

    PianoWidget* piano();
    
    int currentItem() const;

    // needed because we can't call virtual functions in BaseEditor's constructor
    void init();

protected:

    virtual void showEvent(QShowEvent *evt) override;

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

    bool mShown;

};
