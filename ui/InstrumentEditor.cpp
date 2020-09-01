
#include "InstrumentEditor.hpp"

#include "trackerboy/ChType.hpp"
using trackerboy::ChType;


InstrumentEditor::InstrumentEditor(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(mChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstrumentEditor::onChannelSelect);
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
}
