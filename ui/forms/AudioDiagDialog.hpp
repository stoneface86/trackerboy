
#pragma once

#include "core/audio/Renderer.hpp"
#include "core/Config.hpp"
#include "core/Miniaudio.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
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

private slots:
    void refresh();

private:

    Renderer &mRenderer;
    int mTimerId;

    QVBoxLayout mLayout;
        QGroupBox mRenderGroup;
            QFormLayout mRenderLayout;
                QLabel mLockFailsLabel;
                QLabel mUnderrunLabel;
                QLabel mBufferLabel;
                QLabel mStatusLabel;
                QLabel mElapsedLabel;
                QLabel mReturnBufferLabel;
                QPushButton mClearButton;
        QHBoxLayout mButtonLayout;
            QCheckBox mAutoRefreshCheck;
            QSpinBox mIntervalSpin;
            QPushButton mRefreshButton;
            QPushButton mCloseButton;
};
