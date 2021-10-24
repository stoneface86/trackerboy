
#include "forms/AudioDiagDialog.hpp"

#include <QTimerEvent>

constexpr int DEFAULT_REFRESH_INTERVAL = 100;

AudioDiagDialog::AudioDiagDialog(Renderer &renderer, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mRenderer(renderer),
    mTimerId(-1),
    mLayout(),
    mRenderGroup(tr("Render statistics")),
    mRenderLayout(),
    mUnderrunLabel(),
    mBufferProgress(),
    mStatusLabel(),
    mElapsedLabel(),
    mPeriodLabel(),
    mPeriodWrittenLabel(),
    mClearButton(tr("Clear")),
    mButtonLayout(),
    mAutoRefreshCheck(tr("Auto refresh")),
    mIntervalSpin(),
    mRefreshButton(tr("Refresh")),
    mCloseButton(tr("Close"))
{
    mRenderLayout.addRow(tr("Underruns"), &mUnderrunLabel);
    mRenderLayout.addRow(tr("Buffer usage"), &mBufferProgress);
    mRenderLayout.addRow(tr("Status"), &mStatusLabel);
    mRenderLayout.addRow(tr("Elapsed"), &mElapsedLabel);
    mRenderLayout.addRow(tr("Refresh rate"), &mPeriodLabel);
    mRenderLayout.addRow(tr("Samples written"), &mPeriodWrittenLabel);
    mRenderLayout.setWidget(6, QFormLayout::LabelRole, &mClearButton);
    mRenderGroup.setLayout(&mRenderLayout);

    mButtonLayout.addWidget(&mAutoRefreshCheck);
    mButtonLayout.addWidget(&mIntervalSpin);
    mButtonLayout.addWidget(&mRefreshButton);
    mButtonLayout.addStretch();
    mButtonLayout.addWidget(&mCloseButton);

    mLayout.addWidget(&mRenderGroup, 1);
    mLayout.addLayout(&mButtonLayout);
    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(&mLayout);

    mIntervalSpin.setSuffix(tr(" ms"));
    mIntervalSpin.setRange(10, 60000);
    mIntervalSpin.setValue(DEFAULT_REFRESH_INTERVAL);

    mAutoRefreshCheck.setCheckState(Qt::Checked);

    mBufferProgress.setAlignment(Qt::AlignCenter);
    mBufferProgress.setFormat(tr("%p% (%v / %m samples)"));

    mCloseButton.setDefault(true);

    setWindowTitle(tr("Audio diagnostics"));

    connect(&mCloseButton, &QPushButton::clicked, this, &AudioDiagDialog::close);
    connect(&mRefreshButton, &QPushButton::clicked, this, &AudioDiagDialog::refresh);
    connect(&mClearButton, &QPushButton::clicked, &mRenderer, &Renderer::clearDiagnostics);
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
    Q_UNUSED(evt);

    if (mTimerId != -1) {
        killTimer(mTimerId);
        mTimerId = -1;
    }
}

void AudioDiagDialog::showEvent(QShowEvent *evt) {
    Q_UNUSED(evt);

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

    auto diags = mRenderer.diagnostics();

    mUnderrunLabel.setText(QString::number(diags.underruns));

    mBufferProgress.setMaximum((int)diags.bufferSize);
    mBufferProgress.setValue((int)diags.bufferUse);
    /*mBufferLabel.setText(tr("%1% (%2 / %3 samples)")
        .arg(diags.bufferUse * 100 / diags.bufferSize)
        .arg(diags.bufferUse)
        .arg(diags.bufferSize)
    );*/

    auto msecs = (long)(diags.elapsed * 1000.0);

    mElapsedLabel.setText(QStringLiteral("%1:%2.%3")
        .arg(msecs / 60000, 2, 10, QChar('0'))
        .arg((msecs % 60000) / 1000, 2, 10, QChar('0'))
        .arg(msecs % 1000, 3, 10, QChar('0'))
        );

    mStatusLabel.setText(mRenderer.isRunning() ? tr("Playing") : tr("Stopped"));

    double periodMs = std::chrono::duration<double>(diags.lastPeriod).count() * 1000.0;
    mPeriodLabel.setText(tr("%1 ms").arg(periodMs, 0, 'f', 3));
    mPeriodWrittenLabel.setText(QString::number(diags.writesSinceLastPeriod));
}
