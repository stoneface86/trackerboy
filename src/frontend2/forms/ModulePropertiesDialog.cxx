
#include "forms/ModulePropertiesDialog.hxx"
#include "backend.hxx"
#include "utils/connectutils.hxx"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

ModulePropertiesDialog::ModulePropertiesDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                          Qt::WindowCloseButtonHint)
    , mLineTitle{}
    , mLineArtist{}
    , mLineCopyright{}
    , mTickrate{}
    , mRevisionLabel{}
    , mSaveButton{} {

    setWindowTitle(tr("Module Properties"));
    auto layout = new QVBoxLayout;

    auto infoGroup = new QGroupBox(tr("Information"));
    auto infoLayout = new QVBoxLayout;
    auto makeInfoStringEdit = [](QString const &placeholder) -> QLineEdit * {
        QLineEdit *result = new QLineEdit;
        result->setPlaceholderText(placeholder);
        result->setMaxLength(32);
        return result;
    };
    mLineTitle = makeInfoStringEdit(tr("Title"));
    mLineArtist = makeInfoStringEdit(tr("Artist"));
    mLineCopyright = makeInfoStringEdit(tr("Copyright"));
    infoLayout->addWidget(mLineTitle);
    infoLayout->addWidget(mLineArtist);
    infoLayout->addWidget(mLineCopyright);
    infoGroup->setLayout(infoLayout);

    mTickrate = new TickrateForm;

    mRevisionLabel = new QLabel;

    auto dialogButtons =
        new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    mSaveButton = dialogButtons->button(QDialogButtonBox::Save);
    mSaveButton->setEnabled(false);

    layout->addWidget(infoGroup);
    layout->addWidget(mTickrate);
    layout->addWidget(mRevisionLabel);
    layout->addWidget(dialogButtons);
    setLayout(layout);

    lazyconnect(dialogButtons, accepted, this, accept);
    lazyconnect(dialogButtons, rejected, this, reject);
    lazyconnect(mLineTitle, textChanged, this, setDirty);
    lazyconnect(mLineArtist, textChanged, this, setDirty);
    lazyconnect(mLineCopyright, textChanged, this, setDirty);
    lazyconnect(mTickrate, tickrateChanged, this, setDirty);
}

void ModulePropertiesDialog::load(Document const &doc) {
    auto const props = doc.view()->mod.moduleProperties();
    auto loadInfoString = [](QLineEdit *edit, const char *data) {
        auto len = 0;
        do {
            if (data[len] == '\0') {
                break;
            }
        } while (++len < B::InfoStringLen);
        edit->setText(QString::fromUtf8(data, len));
    };
    loadInfoString(mLineTitle, props.title);
    loadInfoString(mLineArtist, props.artist);
    loadInfoString(mLineCopyright, props.copyright);
    mTickrate->setTickrate(props.tickrate);
    mRevisionLabel->setText(tr("Module Revision: %1.%1")
                                .arg((int)props.revMajor, (int)props.revMinor));
    mSaveButton->setEnabled(false);
}

void ModulePropertiesDialog::save(Document &doc) const {
    B::ModuleProperties props;

    auto setInfoString = [](QLineEdit *edit, char *out) {
        auto tmp = edit->text().toUtf8();
        int i = 0;
        int const tmpLen = qMin((int)tmp.size(), (int)B::InfoStringLen);
        for (; i < tmpLen; ++i) {
            out[i] = tmp[i];
        }
        for (; i < (int)B::InfoStringLen; ++i) {
            out[i] = '\0';
        }
    };
    setInfoString(mLineTitle, props.title);
    setInfoString(mLineArtist, props.artist);
    setInfoString(mLineCopyright, props.copyright);
    props.tickrate = mTickrate->tickrate();
    doc.edit(true)->mod.setModuleProperties(props);
    mSaveButton->setEnabled(false);
}

void ModulePropertiesDialog::setDirty() {
    mSaveButton->setEnabled(true);
}
