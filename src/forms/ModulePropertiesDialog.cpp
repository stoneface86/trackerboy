
#include "forms/ModulePropertiesDialog.hpp"
#include "utils/connectutils.hpp"

#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QListView>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QMessageBox>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QtDebug>

// Note: this model subclass (commented out) was for editing the song list
// out of place, or changes made to the list would only occur after the
// user accepts the dialog (hits OK). The current implementation edits the
// list in place, or changes take effect immediately.
// In order to support out of place, the SongList class in trackerboy would
// need to be rewritten.
//
// Custom model used internally by the dialog for editing the song list.
// Supports adding, removing, and duplicating songs, as well as reordering
// and renaming. Modifications made do not take effect until the user accepts
// the dialog.
//
//class ModulePropertiesDialog::CustomModel : public QIdentityProxyModel {
//    Q_OBJECT

//public:
//    explicit CustomModel(Module const& mod, QObject *parent = nullptr) :
//        QIdentityProxyModel(parent),
//        mModifications()
//    {
//        mSourceModel = new QStringListModel(this);
//        auto numproxy = new NumberedProxyModel(this);
//        numproxy->setSourceModel(mSourceModel);
//        setSourceModel(numproxy);

//        // this class boldens newly added songs and duplicates
//        // numproxy adds a number to the name
//        // and the source model is just a simple QStringListModel of song names

//        auto const& songlist = mod.data().songs();
//        auto const songcount = songlist.size();
//        QStringList names;
//        for (int i = 0; i < songcount; ++i) {
//            names.append(QString::fromStdString(songlist.get(i)->name()));
//            mModifications.push_back(i);
//        }
//        mSourceModel->setStringList(names);
//    }

//    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override {
//        if (role != Qt::FontRole) {
//            return QIdentityProxyModel::data(index, role);
//        }

//        QFont font;
//        auto mod = mModifications[index.row()];
//        if (mod == -1 || mod > 255) {
//            // new songs are -1
//            // duplicates are the source of the duplicated index + 256
//            font.setBold(true);
//        }
//        return font;
//    }

//    void add() {
//        mModifications.append(-1);
//        int row = mSourceModel->rowCount();
//        mSourceModel->insertRow(row);
//        mSourceModel->setData(mSourceModel->index(row), tr("New song"));
//    }

//    void remove(int row) {
//        mSourceModel->removeRow(row);
//        mModifications.remove(row);
//    }

//    void duplicate(int row) {
//        // works for both new songs (-1) and existing ones
//        mModifications.insert(row + 1, 0x100 | mModifications[row]);

//        auto index = mSourceModel->index(row);
//        mSourceModel->insertRow(row);
//        mSourceModel->setData(index, mSourceModel->stringList()[row + 1]);
//    }

//    void moveUp(int row) {
//        auto const dest = row - 1;
//        mSourceModel->moveRow(QModelIndex(), row, QModelIndex(), dest);
//        mModifications.swapItemsAt(row, dest);
//    }

//    void moveDown(int row) {
//        auto const dest = row + 1;
//        mSourceModel->moveRow(QModelIndex(), row, QModelIndex(), dest + 1);
//        mModifications.swapItemsAt(row, dest);
//    }

//    void applyChanges(Module &mod) {
//        // first check for any song additions/removals
//        // if the modifications vector is the same size of the song list,
//        // and its indices go from 0 to size - 1 (ie { 0, 1, 2}) then
//        // we don't need to update it

//        auto &songs = mod.data().songs();

//        if (songs.size() != mModifications.size()) {

//            bool updateList = true;
//            int i = 0;
//            for (auto index : mModifications) {
//                if (i != index) {
//                    updateList = false;
//                    break;
//                }
//            }

//            if (updateList) {
//                trackerboy::SongList::Container container;
//                for (auto modification : mModifications) {
//                    if (modification == -1) {
//                        // push a new song
//                        container.emplace_back(std::make_shared<trackerboy::Song>());
//                    } else if (modification > 0xFF) {
//                        // duplicate
//                        int index = modification & 0xFF;
//                        container.emplace_back(std::make_shared<trackerboy::Song>(*songs.get(index)));
//                    } else {
//                        // use existing
//                        container.push_back(songs.getShared(modification));
//                    }
//                }
//            }


//        }




//        // update names
//        auto const names = mSourceModel->stringList();
//        for (auto const& name : names) {
//            (*iter++)->setName(name.toStdString());
//        }



//    }

//private:

//    QStringListModel *mSourceModel;

//    // list of indices representing the modifications to the song list
//    // if our song list has 3 songs: { A, B, C }
//    // for no modifications: mModifications = { 0, 1, 2 }
//    // remove B, duplicate C, add new song D: { 0, 2, 0x102, -1 }
//    QVector<int> mModifications;
//};


ModulePropertiesDialog::ModulePropertiesDialog(Module &mod, SongListModel &model, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mModule(mod),
    mModel(model),
    mDirty(Clean),
    mWarnRemove(true),
    mSongList(new QListView),
    mAddButton(new QPushButton(tr("Add"))),
    mRemoveButton(new QPushButton(tr("Remove"))),
    mDuplicateButton(new QPushButton(tr("Duplicate"))),
    mMoveUpButton(new QPushButton(tr("Move up"))),
    mMoveDownButton(new QPushButton(tr("Move down"))),
    mTitleEdit(new QLineEdit),
    mArtistEdit(new QLineEdit),
    mCopyrightEdit(new QLineEdit),
    mCustomSpin(new QSpinBox),
    mDialogButtons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)),
    mFramerateRadios(new QButtonGroup(this))
{
    setModal(true);
    setWindowTitle(tr("Module properties"));


    auto layout = new QVBoxLayout;

    // song list group
    auto songListGroup = new QGroupBox(tr("Song list"));
    auto songListLayout = new QHBoxLayout;
    mSongList->setModel(&model);

    auto songButtonLayout = new QVBoxLayout;
    songButtonLayout->addWidget(mAddButton);
    songButtonLayout->addWidget(mRemoveButton);
    songButtonLayout->addWidget(mDuplicateButton);
    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    songButtonLayout->addWidget(line);
    songButtonLayout->addWidget(mMoveUpButton);
    songButtonLayout->addWidget(mMoveDownButton);
    songButtonLayout->addStretch();

    songListLayout->addWidget(mSongList, 1);
    songListLayout->addLayout(songButtonLayout);
    songListGroup->setLayout(songListLayout);

    auto propLayout = new QHBoxLayout;

    auto infoGroup = new QGroupBox(tr("Information"));
    auto infoLayout = new QVBoxLayout;
    infoLayout->addWidget(mTitleEdit);
    infoLayout->addWidget(mArtistEdit);
    infoLayout->addWidget(mCopyrightEdit);
    infoGroup->setLayout(infoLayout);

    auto framerateGroup = new QGroupBox(tr("Framerate"));
    auto framerateLayout = new QVBoxLayout;
    auto dmgRadio = new QRadioButton(tr("DMG (59.7 Hz)"));
    auto sgbRadio = new QRadioButton(tr("SGB (61.1 Hz)"));
    auto customLayout = new QHBoxLayout;
    auto customRadio = new QRadioButton(tr("Custom"));
    mCustomSpin->setRange(15, 4096); // theres an issue with < 15 Hz framerates
    mCustomSpin->setSuffix(tr(" Hz"));
    customLayout->addWidget(customRadio);
    customLayout->addWidget(mCustomSpin);
    customLayout->addStretch();
    framerateLayout->addWidget(dmgRadio);
    framerateLayout->addWidget(sgbRadio);
    framerateLayout->addLayout(customLayout);
    framerateGroup->setLayout(framerateLayout);

    propLayout->addWidget(infoGroup, 1);
    propLayout->addWidget(framerateGroup);


    layout->addWidget(songListGroup, 1);
    layout->addLayout(propLayout);
    auto moduleVersionLabel = new QLabel(tr("Module revision: %1.%2").arg(
                                             QString::number(mod.data().revisionMajor()),
                                             QString::number(mod.data().revisionMinor())));

    mDialogButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    lazyconnect(mDialogButtons, accepted, this, accept);
    lazyconnect(mDialogButtons, rejected, this, reject);

    layout->addWidget(moduleVersionLabel);
    layout->addWidget(mDialogButtons);

    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);


    mTitleEdit->setPlaceholderText(tr("Title"));
    mArtistEdit->setPlaceholderText(tr("Artist"));
    mCopyrightEdit->setPlaceholderText(tr("Copyright"));

    auto selectionModel = mSongList->selectionModel();
    lazyconnect(selectionModel, selectionChanged, this, updateSongButtons);

    auto const& moddata = mModule.data();
    auto setInfoEdit = [](QLineEdit *edit, trackerboy::InfoStr const& str) {
        edit->setMaxLength((int)str.size());
        edit->setText(QString::fromUtf8(str.data(), str.length()));
    };
    setInfoEdit(mTitleEdit, moddata.title());
    setInfoEdit(mArtistEdit, moddata.artist());
    setInfoEdit(mCopyrightEdit, moddata.copyright());

    switch (moddata.system()) {
        case trackerboy::System::dmg:
            dmgRadio->setChecked(true);
            break;
        case trackerboy::System::sgb:
            sgbRadio->setChecked(true);
            break;
        case trackerboy::System::custom:
            customRadio->setChecked(true);
            break;
    }
    mCustomSpin->setValue(moddata.customFramerate());
    mCustomSpin->setEnabled(customRadio->isChecked());



    mFramerateRadios->addButton(dmgRadio, (int)trackerboy::System::dmg);
    mFramerateRadios->addButton(sgbRadio, (int)trackerboy::System::sgb);
    mFramerateRadios->addButton(customRadio, (int)trackerboy::System::custom);


    updateSongButtons();

    lazyconnect(mAddButton, clicked, this, add);
    lazyconnect(mRemoveButton, clicked, this, remove);
    lazyconnect(mDuplicateButton, clicked, this, duplicate);
    lazyconnect(mMoveUpButton, clicked, this, moveUp);
    lazyconnect(mMoveDownButton, clicked, this, moveDown);
    lazyconnect(customRadio, toggled, mCustomSpin, setEnabled);

    auto setInfoDirty = [this]() { setDirty(DirtyInfo); };
    connect(mTitleEdit, &QLineEdit::textChanged, this, setInfoDirty);
    connect(mArtistEdit, &QLineEdit::textChanged, this, setInfoDirty);
    connect(mCopyrightEdit, &QLineEdit::textChanged, this, setInfoDirty);

    auto setSystemDirty = [this]() { setDirty(DirtySystem); };
    connect(mFramerateRadios, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, setSystemDirty);
    connect(mCustomSpin, qOverload<int>(&QSpinBox::valueChanged), this, setSystemDirty);

}

void ModulePropertiesDialog::accept() {
    int ret = Accepted;

    if (mDirty) {
        // write changes to the module
        auto editor = mModule.permanentEdit();

        auto &mod = mModule.data();

        // info strings
        if (mDirty & DirtyInfo) {

            // note there is an implicit conversion from std::string to trackerboy::InfoStr
            mod.setTitle(mTitleEdit->text().toStdString());
            mod.setArtist(mArtistEdit->text().toStdString());
            mod.setCopyright(mCopyrightEdit->text().toStdString());
        }


        // system
        if (mDirty & DirtySystem) {
            auto system = (trackerboy::System)mFramerateRadios->checkedId();
            switch (system) {
                case trackerboy::System::dmg:
                case trackerboy::System::sgb:
                    if (system != mod.system()) {
                        mod.setFramerate(system);
                        ret = AcceptedSystemChange;
                    }
                    break;
                default: // custom
                    if (system != mod.system() || mCustomSpin->value() != mod.customFramerate()) {
                        mod.setFramerate(mCustomSpin->value());
                        ret = AcceptedSystemChange;
                    }
                    break;
            }
        }

    }

    done(ret);
}

int ModulePropertiesDialog::selectedSong() {
    auto selection = mSongList->selectionModel()->selection();
    if (selection.empty()) {
        return -1;
    } else {
        // this shit is so stupid for single-selection 1D models
        return selection.first().topLeft().row();
    }
}

void ModulePropertiesDialog::add() {
    mModel.append();
    updateSongButtons();
}

void ModulePropertiesDialog::remove() {

    auto index = selectedSong();

    if (mWarnRemove) {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Question);
        msgbox.setText(tr("Remove song '%1' ?").arg(mModel.data(mModel.index(index)).toString()));
        msgbox.setInformativeText(tr("This operation cannot be undone."));
        msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        QCheckBox warnCheck(tr("Prompt me when removing songs"));
        warnCheck.setChecked(true);
        msgbox.setCheckBox(&warnCheck);

        int ret = msgbox.exec();
        msgbox.setCheckBox(nullptr); // so msgbox doesn't delete this later


        mWarnRemove = warnCheck.isChecked();

        if (ret == QMessageBox::No) {
            return;
        }
    }

    mModel.remove(selectedSong());
    updateSongButtons();
}

void ModulePropertiesDialog::duplicate() {
    mModel.duplicate(selectedSong());

    updateSongButtons();
}

void ModulePropertiesDialog::moveUp() {

    mModel.moveUp(selectedSong());

    updateSongButtons();
}

void ModulePropertiesDialog::moveDown() {

    mModel.moveDown(selectedSong());

    updateSongButtons();
}


void ModulePropertiesDialog::setDirty(DirtyFlag flag) {
    if (mDirty == 0) {
        mDialogButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    mDirty |= flag;
}

void ModulePropertiesDialog::updateSongButtons() {
    auto const count = mModel.rowCount();
    bool const canAdd = count < 256;
    auto const _selectedSong = selectedSong();
    bool const hasSelection = _selectedSong != -1;
    mAddButton->setEnabled(canAdd);
    mRemoveButton->setEnabled(count > 1 && hasSelection);
    mDuplicateButton->setEnabled(canAdd && hasSelection);
    mMoveUpButton->setEnabled(_selectedSong > 0);
    mMoveDownButton->setEnabled(hasSelection && _selectedSong != count - 1);
}

//#include "ModulePropertiesDialog.moc"
