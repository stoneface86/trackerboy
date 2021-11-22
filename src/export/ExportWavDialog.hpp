
#pragma once

#include "core/Module.hpp"
#include "core/ModuleFile.hpp"
#include "export/WavExporter.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>

class ExportWavDialog : public QDialog {

    Q_OBJECT

public:

    explicit ExportWavDialog(
        Module const& mod,
        ModuleFile const& modFile,
        int samplerate,
        QWidget *parent = nullptr
    );

    virtual void accept() override;

    virtual void reject() override;

private:
    void setGroupsEnabled(bool enabled);

    Module const& mModule;
    int mSamplerate;

    QVBoxLayout mLayout;
        QGroupBox mDurationGroup;
            QGridLayout mDurationLayout;
                QRadioButton mLoopRadio;
                QSpinBox mLoopSpin;
                QLabel mLoopLabel;
                QRadioButton mTimeRadio;
                QLineEdit mTimeEdit;
                QLabel mTimeLabel;
        QGroupBox mChannelGroup;
            QVBoxLayout mChannelLayout;
                QHBoxLayout mChannelSelectLayout;
                    std::array<QCheckBox, 4> mChannelChecks;
                QCheckBox mSeparateChannelsCheck;
        QGroupBox mDestinationGroup;
            QHBoxLayout mDestinationLayout;
                QLineEdit mFilenameEdit;
                QPushButton mBrowseButton;
        QProgressBar mProgress;
        QLabel mStatusLabel;
        QDialogButtonBox mButtons;

        QPushButton *mExportButton;

    WavExporter *mExporter;

    unsigned mTimeEditDuration;

};
