
#pragma once

#include "core/Module.hpp"
#include "core/model/SongListModel.hpp"

#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSpinBox>


class ModulePropertiesDialog : public QDialog {

    Q_OBJECT

public:

    //
    // exec() returns this value if the user changed the framerate/system
    //
    static constexpr int AcceptedSystemChange = 2;

    explicit ModulePropertiesDialog(Module &mod, SongListModel &model, QWidget *parent = nullptr);

    virtual void accept() override;

private:

    enum DirtyFlag {
        Clean = 0x0,
        DirtyInfo = 0x1,
        DirtySystem = 0x2
    };


    Q_DISABLE_COPY(ModulePropertiesDialog)

    int selectedSong();

    void add();

    void remove();

    void duplicate();

    void moveUp();

    void moveDown();


    void setDirty(DirtyFlag flag);

    void updateSongButtons();

    Module &mModule;
    SongListModel &mModel;
    unsigned mDirty;

    bool mWarnRemove;

    QListView *mSongList;

    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mDuplicateButton;
    QPushButton *mMoveUpButton;
    QPushButton *mMoveDownButton;

    QLineEdit *mTitleEdit;
    QLineEdit *mArtistEdit;
    QLineEdit *mCopyrightEdit;

    QSpinBox *mCustomSpin;

    QDialogButtonBox *mDialogButtons;

    QButtonGroup *mFramerateRadios;


};
