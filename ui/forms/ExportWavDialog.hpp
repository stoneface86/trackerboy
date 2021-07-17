
#pragma once

class Document;
class Config;
class WavExporter;

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

    explicit ExportWavDialog(Document &document, Config &config, QWidget *parent = nullptr);

    virtual void accept() override;

    virtual void reject() override;

private:
    void setGroupsEnabled(bool enabled);

    Document &mDocument;
    Config &mConfig;

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
