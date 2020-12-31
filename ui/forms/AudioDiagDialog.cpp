
#include "forms/AudioDiagDialog.hpp"

#include <QTime>
#include <QTimerEvent>

constexpr int DEFAULT_REFRESH_INTERVAL = 100;

AudioDiagDialog::AudioDiagDialog(Renderer &renderer, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mRenderer(renderer),
    mTimerId(-1),
    mLayout(),
    mRenderGroup(tr("Render statistics")),
    mRenderLayout(),
    mLockFailsLabel(),
    mUnderrunLabel(),
    mBufferLabel(),
    mStatusLabel(),
    mElapsedLabel(),
    mClearButton(tr("Clear")),
    mDeviceGroup(tr("Device information")),
    mDeviceLayout(),
    mDeviceNameLabel(),
    mBackendLabel(),
    mDeviceIdEdit(),
    mButtonLayout(),
    mAutoRefreshCheck(tr("Auto refresh")),
    mIntervalSpin(),
    mRefreshButton(tr("Refresh")),
    mCloseButton(tr("Close"))
{

    mRenderLayout.addRow(tr("Lock fails"), &mLockFailsLabel);
    mRenderLayout.addRow(tr("Underruns"), &mUnderrunLabel);
    mRenderLayout.addRow(tr("Buffer utilization"), &mBufferLabel);
    mRenderLayout.addRow(tr("Status"), &mStatusLabel);
    mRenderLayout.addRow(tr("Elapsed"), &mElapsedLabel);
    mRenderLayout.setWidget(5, QFormLayout::LabelRole, &mClearButton);
    mRenderGroup.setLayout(&mRenderLayout);

    mDeviceLayout.addRow(tr("Device name: "), &mDeviceNameLabel);
    mDeviceLayout.addRow(tr("Backend: "), &mBackendLabel);
    mDeviceLayout.addRow(tr("Device ID:"), &mDeviceIdEdit);
    mDeviceGroup.setLayout(&mDeviceLayout);

    mButtonLayout.addWidget(&mAutoRefreshCheck);
    mButtonLayout.addWidget(&mIntervalSpin);
    mButtonLayout.addWidget(&mRefreshButton);
    mButtonLayout.addStretch();
    mButtonLayout.addWidget(&mCloseButton);
    
    mGroupLayout.addWidget(&mRenderGroup);
    mGroupLayout.addWidget(&mDeviceGroup, 1);

    mLayout.addLayout(&mGroupLayout, 1);
    mLayout.addLayout(&mButtonLayout);
    setLayout(&mLayout);

    mIntervalSpin.setSuffix(tr(" ms"));
    mIntervalSpin.setRange(10, 60000);
    mIntervalSpin.setValue(DEFAULT_REFRESH_INTERVAL);

    mDeviceIdEdit.setReadOnly(true);

    mAutoRefreshCheck.setCheckState(Qt::Checked);

    setWindowTitle(tr("Audio diagnostics"));

    connect(&mCloseButton, &QPushButton::clicked, this, &AudioDiagDialog::close);
    connect(&mRefreshButton, &QPushButton::clicked, this, &AudioDiagDialog::refresh);
    connect(&mAutoRefreshCheck, &QCheckBox::stateChanged, this,
        [this](int state) {
            bool checked = state == Qt::Checked;
            mIntervalSpin.setEnabled(checked);
            if (checked) {
                mTimerId = startTimer(mIntervalSpin.value());
            } else {
                killTimer(mTimerId);
            }
        });
    connect(&mIntervalSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            if (mTimerId != -1) {
                killTimer(mTimerId);
                mTimerId = startTimer(value);
            }
        });
    
}

AudioDiagDialog::~AudioDiagDialog() {

}

void AudioDiagDialog::setConfig(Miniaudio &miniaudio, Config::Sound const& config) {
    auto deviceIndex = config.deviceIndex - 1;
    mDeviceNameLabel.setText(miniaudio.deviceName(config.deviceIndex - 1));
    mBackendLabel.setText(miniaudio.backendName());
    mDeviceIdEdit.setText(miniaudio.deviceIdString(deviceIndex));
    mBuffersize = config.buffersize;
}

void AudioDiagDialog::closeEvent(QCloseEvent *evt) {
    if (mTimerId != -1) {
        killTimer(mTimerId);
        mTimerId = -1;
    }
}

void AudioDiagDialog::showEvent(QShowEvent *evt) {
    if (mAutoRefreshCheck.isChecked() && mTimerId == -1) {
        mTimerId = startTimer(mIntervalSpin.value());
    }
}

void AudioDiagDialog::timerEvent(QTimerEvent *evt) {
    if (evt->timerId() == mTimerId) {
        refresh();
    }
}

void AudioDiagDialog::refresh() {
    mLockFailsLabel.setText(QString::number(mRenderer.lockFails()));
    mUnderrunLabel.setText(QString::number(mRenderer.underruns()));
    mBufferLabel.setText(QStringLiteral("%1 / %2").arg(mRenderer.bufferUsage()).arg(mBuffersize));

    // assumes 48000 samplerate
    auto elapsed = mRenderer.elapsed() / 48;
    mElapsedLabel.setText(QStringLiteral("%1:%2.%3")
        .arg(elapsed / 60000, 2, 10, QChar('0'))
        .arg((elapsed % 60000) / 1000, 2, 10, QChar('0'))
        .arg(elapsed % 1000, 3, 10, QChar('0'))
        );
}
