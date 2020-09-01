
#include "InstrumentEditor.hpp"

#include "trackerboy/ChType.hpp"
using trackerboy::ChType;


InstrumentEditor::InstrumentEditor(QWidget *parent) :
    mUpdating(false),
    QDialog(parent)
{
    setupUi(this);

    QFont font = mEnvelopeSpin->font();
    font.setCapitalization(QFont::AllUppercase);
    mEnvelopeSpin->setFont(font);

    connect(mChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstrumentEditor::onChannelSelect);
    connect(mEnvelopeInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentEditor::envelopeEditChanged);
    connect(mEnvelopeIncreasingCheckbox, &QCheckBox::stateChanged, this, &InstrumentEditor::envelopeEditChanged);
    connect(mEnvelopePeriodSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentEditor::envelopeEditChanged);
    connect(mEnvelopeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentEditor::envelopeChanged);

}


void InstrumentEditor::onChannelSelect(int channel) {
    // disable controls based on the selected channel
    bool isFrequencyChannel = channel != static_cast<int>(ChType::ch4);
    mGroupFrequency->setEnabled(isFrequencyChannel);

    bool isWaveChannel = channel == static_cast<int>(ChType::ch3);
    mEnvelopeInitVolumeSpin->setEnabled(!isWaveChannel);
    mEnvelopeIncreasingCheckbox->setEnabled(!isWaveChannel);
    mEnvelopePeriodSpin->setEnabled(!isWaveChannel);

    mWaveCombo->setEnabled(isWaveChannel);
    mWaveEditButton->setEnabled(isWaveChannel);

    // update the current instrument's channel
}

void InstrumentEditor::envelopeChanged(int value) {
    (void)value;

    // this slot is either called directly by the user changing the envelope spinbox or
    // indirectly by setting the value programmatically from the user editing any of the
    // envelope editor controls. The mUpdating member prevents infinite recursion from
    // changing both controls.

    if (!mUpdating) {
        mUpdating = true;
        int envelope = mEnvelopeSpin->value();
        mEnvelopeInitVolumeSpin->setValue(envelope >> 4);
        mEnvelopeIncreasingCheckbox->setChecked(!!(envelope & 0x8));
        mEnvelopePeriodSpin->setValue(envelope & 0x7);
        mUpdating = false;
    }
    
    // update the current instrument's envelope
}

void InstrumentEditor::envelopeEditChanged(int value) {
    (void)value;
    
    if (!mUpdating) {
        mUpdating = true;
        int envelope = mEnvelopeInitVolumeSpin->value() << 4;
        if (mEnvelopeIncreasingCheckbox->isChecked()) {
            envelope |= 0x8;
        }
        envelope |= mEnvelopePeriodSpin->value();
        mEnvelopeSpin->setValue(envelope);
        mUpdating = false;
    }
}

