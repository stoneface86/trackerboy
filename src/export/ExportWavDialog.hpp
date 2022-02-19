
#pragma once

class Module;
class ModuleFile;
class WavExporter;

class QCheckBox;
#include <QDialog>
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedLayout;

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
    WavExporter *mExporter;
    unsigned mTimeEditDuration;

    QGroupBox *mDurationGroup;
    QGroupBox *mChannelsGroup;
    QGroupBox *mDestinationGroup;

    QRadioButton *mLoopRadio;
    QRadioButton *mTimeRadio;
    QSpinBox *mLoopSpin;
    QLineEdit *mTimeEdit;
    std::array<QCheckBox*, 4> mChannelChecks;

    QCheckBox *mSeparateChannelsCheck;
    QStackedLayout *mDestinationStack;
    QLineEdit *mSingleDestination;
    QLineEdit *mSeparateDestination;
    QLineEdit *mSeparatePrefix;

    QProgressBar *mProgress;
    QLabel *mStatusLabel;
    QPushButton *mExportButton;

    

};
