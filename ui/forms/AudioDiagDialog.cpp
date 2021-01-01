
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

    mButtonLayout.addWidget(&mAutoRefreshCheck);
    mButtonLayout.addWidget(&mIntervalSpin);
    mButtonLayout.addWidget(&mRefreshButton);
    mButtonLayout.addStretch();
    mButtonLayout.addWidget(&mCloseButton);

    mLayout.addWidget(&mRenderGroup, 1);
    mLayout.addLayout(&mButtonLayout);
    setLayout(&mLayout);

    mIntervalSpin.setSuffix(tr(" ms"));
    mIntervalSpin.setRange(10, 60000);
    mIntervalSpin.setValue(DEFAULT_REFRESH_INTERVAL);

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
    refresh();
}

void AudioDiagDialog::timerEvent(QTimerEvent *evt) {
    if (evt->timerId() == mTimerId) {
        refresh();
    }
}

void AudioDiagDialog::refresh() {
    mLockFailsLabel.setText(QString::number(mRenderer.lockFails()));
    mUnderrunLabel.setText(QString::number(mRenderer.underruns()));
    mBufferLabel.setText(QStringLiteral("%1 / %2").arg(mRenderer.bufferUsage()).arg(mRenderer.bufferSize()));

    auto elapsed = mRenderer.elapsed();
    mElapsedLabel.setText(QStringLiteral("%1:%2.%3")
        .arg(elapsed / 60000, 2, 10, QChar('0'))
        .arg((elapsed % 60000) / 1000, 2, 10, QChar('0'))
        .arg(elapsed % 1000, 3, 10, QChar('0'))
        );

    mStatusLabel.setText(mRenderer.isRunning() ? tr("Playing") : tr("Stopped"));
}
