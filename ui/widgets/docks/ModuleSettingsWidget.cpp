
#include "widgets/docks/ModuleSettingsWidget.hpp"

ModuleSettingsWidget::ModuleSettingsWidget(QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mLayout(),
    mSettingsLayout(),
    mInfoGroup(tr("Information")),
    mInfoLayout(),
    mTitleEdit(),
    mArtistEdit(),
    mCopyrightEdit(),
    mEngineGroup(tr("Engine settings")),
    mEngineLayout(),
    mFramerateChoiceLayout(),
    mFramerateDmgRadio(tr("DMG (59.7 Hz)")),
    mFramerateSgbRadio(tr("SGB (61.1 Hz)")),
    mFramerateCustomLayout(),
    mFramerateCustomRadio(tr("Custom")),
    mFramerateCustomSpin(),
    mCommentsGroup(tr("Comments")),
    mCommentsLayout(),
    mCommentsEdit(),
    mFramerateButtons()
{
    // layout

    mInfoLayout.addRow(tr("Title:"), &mTitleEdit);
    mInfoLayout.addRow(tr("Artist:"), &mArtistEdit);
    mInfoLayout.addRow(tr("Copyright:"), &mCopyrightEdit);
    mInfoGroup.setLayout(&mInfoLayout);


    mFramerateCustomLayout.addWidget(&mFramerateCustomRadio);
    mFramerateCustomLayout.addWidget(&mFramerateCustomSpin);
    mFramerateCustomLayout.addStretch();

    mFramerateChoiceLayout.addWidget(&mFramerateDmgRadio);
    mFramerateChoiceLayout.addWidget(&mFramerateSgbRadio);
    mFramerateChoiceLayout.addLayout(&mFramerateCustomLayout);

    mEngineLayout.setLabelAlignment(Qt::AlignTop | Qt::AlignLeft);
    mEngineLayout.setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    mEngineLayout.addRow(tr("Framerate"), &mFramerateChoiceLayout);
    // jank incoming (force top alignment on the label created by QFormLayout)
    auto label = static_cast<QLabel*>(mEngineLayout.labelForField(&mFramerateChoiceLayout));
    label->setAlignment(Qt::AlignTop);
    mEngineGroup.setLayout(&mEngineLayout);

    mInfoGroup.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mEngineGroup.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mSettingsLayout.addWidget(&mInfoGroup);
    mSettingsLayout.addWidget(&mEngineGroup);
    mSettingsLayout.addStretch();

    mCommentsLayout.addWidget(&mCommentsEdit, 0, 0);
    mCommentsGroup.setLayout(&mCommentsLayout);

    mLayout.addLayout(&mSettingsLayout);
    mLayout.addWidget(&mCommentsGroup);
    
    setLayout(&mLayout);

    mFramerateButtons.addButton(&mFramerateDmgRadio, static_cast<int>(trackerboy::System::dmg));
    mFramerateButtons.addButton(&mFramerateSgbRadio, static_cast<int>(trackerboy::System::sgb));
    mFramerateButtons.addButton(&mFramerateCustomRadio, static_cast<int>(trackerboy::System::custom));

    // settings

    mTitleEdit.setMaxLength(32);
    mArtistEdit.setMaxLength(32);
    mCopyrightEdit.setMaxLength(32);

    mFramerateCustomSpin.setValue(30);
    mFramerateCustomSpin.setRange(1, 1024);
    mFramerateCustomSpin.setSuffix(tr(" Hz"));

    // connections
    
    connect(&mFramerateButtons, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &ModuleSettingsWidget::framerateButtonToggled);
    connect(&mFramerateCustomSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ModuleSettingsWidget::customFramerateSpinChanged);

    connect(&mTitleEdit, &QLineEdit::textEdited, this, &ModuleSettingsWidget::titleChanged);
    connect(&mArtistEdit, &QLineEdit::textEdited, this, &ModuleSettingsWidget::artistChanged);
    connect(&mCopyrightEdit, &QLineEdit::textEdited, this, &ModuleSettingsWidget::copyrightChanged);
    connect(&mCommentsEdit, &QPlainTextEdit::textChanged, this, &ModuleSettingsWidget::commentsChanged);

    setDocument(nullptr);
}


void ModuleSettingsWidget::titleChanged(QString const& text) {
    if (mDocument) {
        mDocument->setTitle(text);
    }
}

void ModuleSettingsWidget::artistChanged(QString const& text) {
    if (mDocument) {
        mDocument->setArtist(text);
    }
}

void ModuleSettingsWidget::copyrightChanged(QString const& text) {
    if (mDocument) {
        mDocument->setCopyright(text);
    }
}

void ModuleSettingsWidget::commentsChanged() {
    if (mDocument) {
        mDocument->setComments(mCommentsEdit.toPlainText());
    }
}

void ModuleSettingsWidget::setDocument(ModuleDocument *doc) {

    bool const hasDocument = doc != nullptr;

    mDocument = nullptr; // ignores any changes made from loading the current document
    if (hasDocument) {

        // now load settings from the module
        auto const& mod = doc->mod();

        mTitleEdit.setText(doc->title());
        mArtistEdit.setText(doc->artist());
        mCopyrightEdit.setText(doc->copyright());
        mCommentsEdit.setPlainText(doc->comments());

        auto sys = mod.system();
        switch (sys) {
            case trackerboy::System::dmg:
                mFramerateDmgRadio.setChecked(true);
                break;
            case trackerboy::System::sgb:
                mFramerateSgbRadio.setChecked(true);
                break;
            case trackerboy::System::custom:
                mFramerateCustomRadio.setChecked(true);
                break;
        }
        mFramerateCustomSpin.setValue(mod.customFramerate());
        mFramerateCustomSpin.setEnabled(sys == trackerboy::System::custom);

    } else {
        mTitleEdit.clear();
        mArtistEdit.clear();
        mCopyrightEdit.clear();
        mCommentsEdit.clear();
        
    }

    mDocument = doc;
    setEnabled(hasDocument);
    
}

void ModuleSettingsWidget::framerateButtonToggled(QAbstractButton *button, bool checked) {
    if (mDocument && checked) {
        auto sys = static_cast<trackerboy::System>(mFramerateButtons.id(button));

        bool const isCustom = sys == trackerboy::System::custom;
        mFramerateCustomSpin.setEnabled(isCustom);

        auto ctx = mDocument->beginEdit();
        auto& mod = mDocument->mod();
        if (isCustom) {
            mod.setFramerate(mFramerateCustomSpin.value());
        } else {
            mod.setFramerate(sys);
        }

        //calculateActualTempo();
    }
}

void ModuleSettingsWidget::customFramerateSpinChanged(int value) {
    if (mDocument) {
        auto ctx = mDocument->beginEdit();
        mDocument->mod().setFramerate(value);
    }
    //calculateActualTempo();
}
