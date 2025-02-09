
#pragma once

#include "audio/Renderer.hpp"
#include "config/Config.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>

//
// Audio diagnostics dialog. Shows stats about the Renderer and detailed device information
//
class AudioDiagDialog : public QDialog {

    Q_OBJECT

public:

    explicit AudioDiagDialog(Renderer &renderer, QWidget *parent = nullptr);
    ~AudioDiagDialog();

protected:

    void closeEvent(QCloseEvent *evt) override;

    void showEvent(QShowEvent *evt) override;

    void timerEvent(QTimerEvent *evt) override;

private:
    Q_DISABLE_COPY(AudioDiagDialog)

    void refresh();

    void setRunningLabel(bool const isRunning);

    void setElapsed(long const msecs);

    Renderer &mRenderer;
    int mTimerId;
    bool mLastIsRunning;

    QVBoxLayout mLayout;
        QGroupBox mRenderGroup;
            QFormLayout mRenderLayout;
                QLabel mUnderrunLabel;
                //QLabel mBufferLabel;
                QProgressBar mBufferProgress;
                QLabel mStatusLabel;
                QLabel mElapsedLabel;
                QLabel mPeriodLabel;
                QLabel mPeriodWrittenLabel;
                QPushButton mClearButton;
        QHBoxLayout mButtonLayout;
            QCheckBox mAutoRefreshCheck;
            QSpinBox mIntervalSpin;
            QPushButton mRefreshButton;
            QPushButton mCloseButton;
};
