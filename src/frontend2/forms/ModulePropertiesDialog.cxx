
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
    , _lineTitle(nullptr)
    , _lineArtist(nullptr)
    , _lineCopyright(nullptr)
    , _tickrate(nullptr)
    , _revisionLabel(nullptr)
    , _saveButton(nullptr) {

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
    _lineTitle = makeInfoStringEdit(tr("Title"));
    _lineArtist = makeInfoStringEdit(tr("Artist"));
    _lineCopyright = makeInfoStringEdit(tr("Copyright"));
    infoLayout->addWidget(_lineTitle);
    infoLayout->addWidget(_lineArtist);
    infoLayout->addWidget(_lineCopyright);
    infoGroup->setLayout(infoLayout);

    _tickrate = new TickrateForm;

    _revisionLabel = new QLabel;

    auto dialogButtons =
        new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    _saveButton = dialogButtons->button(QDialogButtonBox::Save);
    _saveButton->setEnabled(false);

    layout->addWidget(infoGroup);
    layout->addWidget(_tickrate);
    layout->addWidget(_revisionLabel);
    layout->addWidget(dialogButtons);
    setLayout(layout);

    lazyconnect(dialogButtons, accepted, this, accept);
    lazyconnect(dialogButtons, rejected, this, reject);
    lazyconnect(_lineTitle, textChanged, this, setDirty);
    lazyconnect(_lineArtist, textChanged, this, setDirty);
    lazyconnect(_lineCopyright, textChanged, this, setDirty);
    lazyconnect(_tickrate, tickrateChanged, this, setDirty);
}

void ModulePropertiesDialog::load(Document const &doc) {
    BModuleProperties props;
    doc.view()->mod.moduleProperties(props);
    auto loadInfoString = [](QLineEdit *edit, const char *data) {
        auto len = 0;
        do {
            if (data[len] == '\0') {
                break;
            }
        } while (++len < B::InfoStringLen);
        edit->setText(QString::fromUtf8(data, len));
    };
    loadInfoString(_lineTitle, props.title);
    loadInfoString(_lineArtist, props.artist);
    loadInfoString(_lineCopyright, props.copyright);
    _tickrate->setTickrate(props.tickrate);
    _revisionLabel->setText(tr("Module Revision: %1.%1")
                                .arg((int)props.revMajor, (int)props.revMinor));
    _saveButton->setEnabled(false);
}

void ModulePropertiesDialog::save(Document &doc) const {
    BModuleProperties props;

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
    setInfoString(_lineTitle, props.title);
    setInfoString(_lineArtist, props.artist);
    setInfoString(_lineCopyright, props.copyright);
    props.tickrate = _tickrate->tickrate();
    doc.edit(true)->mod.setModuleProperties(props);
    _saveButton->setEnabled(false);
}

void ModulePropertiesDialog::setDirty() {
    _saveButton->setEnabled(true);
}
