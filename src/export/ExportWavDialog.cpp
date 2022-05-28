
#include "export/ExportWavDialog.hpp"

#include "core/Module.hpp"
#include "core/ModuleFile.hpp"
#include "export/WavExporter.hpp"
#include "verdigris/wobjectimpl.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedLayout>

W_OBJECT_IMPL(ExportWavDialog)

ExportWavDialog::ExportWavDialog(
    Module const& mod,
    ModuleFile const& modFile,
    int samplerate,
    QWidget *parent
) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mModule(mod),
    mSamplerate(samplerate),
    mExporter(nullptr),
    mTimeEditDuration(60)
{
    setModal(true);
    setWindowTitle(tr("Export to WAV"));

    auto layout = new QVBoxLayout;
    mDurationGroup = new QGroupBox(tr("Duration"));
    auto durationLayout = new QGridLayout;
    mLoopRadio = new QRadioButton(tr("Play  the song"));
    mLoopSpin = new QSpinBox;
    mTimeRadio = new QRadioButton(tr("Play for"));
    mTimeEdit = new QLineEdit(QStringLiteral("01:00"));
    durationLayout->addWidget(mLoopRadio, 0, 0);
    durationLayout->addWidget(mLoopSpin, 0, 1);
    durationLayout->addWidget(new QLabel(tr("time(s)")), 0, 2);
    durationLayout->addWidget(mTimeRadio, 1, 0);
    durationLayout->addWidget(mTimeEdit, 1, 1);
    durationLayout->addWidget(new QLabel(tr("mm:ss")), 1, 2);
    mDurationGroup->setLayout(durationLayout);

    mChannelsGroup = new QGroupBox(tr("Channels"));
    auto channelLayout = new QHBoxLayout;
    int ch = 1;
    for (auto &check : mChannelChecks) {
        check = new QCheckBox(QString::number(ch));
        check->setChecked(true);
        channelLayout->addWidget(check);
        connect(check, &QCheckBox::toggled, this,
            [this](bool toggled) {
                if (toggled) {
                    for (auto &box : mChannelChecks) {
                        box->setEnabled(true);
                    }
                } else {
                    int checkCount = 0;
                    QCheckBox *lastChecked = nullptr;
                    for (auto &box : mChannelChecks) {
                        if (box->isChecked()) {
                            ++checkCount;
                            lastChecked = box;
                        }
                    }
                    if (checkCount == 1) {
                        // this will prevent the user from selecting no channels
                        lastChecked->setEnabled(false);
                    }
                }
            }
        );
        ++ch;
    }
    channelLayout->addStretch();
    mChannelsGroup->setLayout(channelLayout);

    mDestinationGroup = new QGroupBox(tr("Destination"));
    auto destinationLayout = new QVBoxLayout;
    mSeparateChannelsCheck = new QCheckBox(tr("Export each channel separately"));
    mDestinationStack = new QStackedLayout;
    destinationLayout->addWidget(mSeparateChannelsCheck);
    destinationLayout->addLayout(mDestinationStack, 1);
    auto singleContainer = new QWidget;
    auto singleLayout = new QHBoxLayout;
    mSingleDestination = new QLineEdit;
    auto browseSingleButton = new QPushButton(tr("Browse"));
    singleLayout->addWidget(mSingleDestination, 1);
    singleLayout->addWidget(browseSingleButton);
    singleLayout->setMargin(0);
    singleLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    singleContainer->setLayout(singleLayout);

    auto separateContainer = new QWidget;
    auto separateLayout = new QGridLayout;
    separateLayout->addWidget(new QLabel(tr("Directory")), 0, 0);
    mSeparateDestination = new QLineEdit;
    separateLayout->addWidget(mSeparateDestination, 0, 1);
    auto browseSeparateButton = new QPushButton(tr("Browse"));
    separateLayout->addWidget(browseSeparateButton, 0, 2);
    separateLayout->addWidget(new QLabel(tr("Prefix")), 1, 0);
    mSeparatePrefix = new QLineEdit;
    separateLayout->addWidget(mSeparatePrefix, 1, 1);
    separateLayout->setMargin(0);
    separateLayout->setColumnStretch(1, 1);
    separateContainer->setLayout(separateLayout);

    mDestinationStack->addWidget(singleContainer);
    mDestinationStack->addWidget(separateContainer);
    mDestinationGroup->setLayout(destinationLayout);

    mProgress = new QProgressBar;
    mStatusLabel = new QLabel;

    auto buttons = new QDialogButtonBox;
    mExportButton = buttons->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
    mExportButton->setEnabled(false);
    buttons->addButton(QDialogButtonBox::Cancel);

    layout->addWidget(mDurationGroup);
    layout->addWidget(mChannelsGroup);
    layout->addWidget(mDestinationGroup);
    layout->addWidget(mProgress);
    layout->addWidget(mStatusLabel);
    layout->addWidget(buttons);
    layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(layout);

    

    mLoopRadio->setChecked(true);
    mLoopSpin->setRange(1, 100);
    mTimeEdit->setInputMask(QStringLiteral("99:99"));
    mTimeEdit->setMaxLength(5);
    mProgress->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    
    connect(buttons, &QDialogButtonBox::accepted, this, &ExportWavDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ExportWavDialog::reject);

    connect(mLoopSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this]() {
            mLoopRadio->setChecked(true);
        });
    
    connect(mTimeEdit, &QLineEdit::textEdited, this,
        [this](QString const& text) {
            mTimeRadio->setChecked(false);

            if (text.length() == 5) {
                QStringRef minRef(&text, 0, 2);
                QStringRef secRef(&text, 3, 2);
                bool ok;
                unsigned duration = 0;
                int num = minRef.toUInt(&ok);
                if (ok) {
                    duration += num * 60;
                }
                num = secRef.toUInt(&ok);
                if (ok) {
                    duration += num;
                }

                if (duration) {
                    mTimeEditDuration = duration;
                    return;
                }
            }
            // whatever the user entered is invalid, restore previous setting
            auto secs = mTimeEditDuration % 60;
            auto mins = mTimeEditDuration / 60;
            mTimeEdit->setText(QStringLiteral("%1:%2")
                            .arg(mins, 2, 10, QChar('0'))
                            .arg(secs, 2, 10, QChar('0')));
        });

    connect(mSingleDestination, &QLineEdit::textChanged, this,
        [this](QString const& str) {
            mExportButton->setEnabled(!str.isEmpty());
        });

    connect(browseSingleButton, &QPushButton::clicked, this,
        [this]() {
            auto filename = QFileDialog::getSaveFileName(
                this,
                tr("Select destination"),
                mSingleDestination->text(),
                tr("WAV files (*.wav)")
            );

            if (filename.isEmpty()) {
                return;
            }

            mSingleDestination->setText(filename);
        });
    connect(browseSeparateButton, &QPushButton::clicked, this,
        [this]() {
            auto path = QFileDialog::getExistingDirectory(
                this,
                tr("Select destination"),
                mSeparateDestination->text()
            );
            if (path.isEmpty()) {
                return;
            }
            mSeparateDestination->setText(path);
        });

    connect(mSeparateChannelsCheck, &QCheckBox::toggled, this,
        [this](bool checked) {
            mDestinationStack->setCurrentIndex(checked ? 1 : 0);
        });

    auto dir = [](ModuleFile const& file) -> QDir {
        if (file.hasFile()) {
            QFileInfo info(file.filepath());
            return info.dir();
        } else {
            return QDir::home();
        }
    }(modFile);
    QString basename = QFileInfo(dir.filePath(modFile.name())).baseName();
    
    mSingleDestination->setText(dir.filePath(basename + ".wav"));
    mSeparateDestination->setText(dir.path());
    mSeparatePrefix->setText(basename);
    
}

void ExportWavDialog::accept() {
    // begin the export
    bool isExporting = false;
    if (mExporter && mExporter->isRunning()) {
        isExporting = true;
    }

    if (!isExporting) {
        if (mExporter == nullptr) {
            mExporter = new WavExporter(mModule, mSamplerate, this);
            connect(mExporter, &WavExporter::progressMax, mProgress, &QProgressBar::setMaximum);
            connect(mExporter, &WavExporter::progress, mProgress, &QProgressBar::setValue);
            connect(mExporter, &WavExporter::finished, this,
                [this]() {
                    if (mExporter->failed()) {
                        mStatusLabel->setText(tr("Export failed"));
                    } else {
                        mProgress->setValue(mProgress->maximum());
                        mStatusLabel->setText(tr("Export complete"));
                    }
                    mExportButton->setEnabled(true);
                    setGroupsEnabled(true);
                });
        }

        if (mLoopRadio->isChecked()) {
            mExporter->setDuration(mLoopSpin->value());
        } else {
            mExporter->setDuration(std::chrono::seconds(mTimeEditDuration));
        }

        {
            ChannelOutput::Flags channels = ChannelOutput::AllOff;
            if (mChannelChecks[0]->isChecked()) {
                channels |= ChannelOutput::CH1;
            }
            if (mChannelChecks[1]->isChecked()) {
                channels |= ChannelOutput::CH2;
            }
            if (mChannelChecks[2]->isChecked()) {
                channels |= ChannelOutput::CH3;
            }
            if (mChannelChecks[3]->isChecked()) {
                channels |= ChannelOutput::CH4;
            }
            mExporter->setChannels(channels);
        }

        if (mSeparateChannelsCheck->isChecked()) {
            mExporter->setSeparate(true);
            mExporter->setDestination(mSeparateDestination->text());
            mExporter->setSeparatePrefix(mSeparatePrefix->text());
        } else {
            mExporter->setSeparate(false);
            mExporter->setDestination(mSingleDestination->text());
        }

        mStatusLabel->setText(tr("Exporting..."));
        mProgress->setValue(0);
        setGroupsEnabled(false);
        mExporter->start();
        mExportButton->setEnabled(false);
    }

    // don't call QDialog::accept, we want to keep the dialog open until the user
    // closes it
}

void ExportWavDialog::reject() {
    // cancel the current export if there is one in progress
    if (mExporter && mExporter->isRunning()) {
        mExporter->cancel();
        mExporter->wait();
    }

    QDialog::reject();
}

void ExportWavDialog::setGroupsEnabled(bool enabled) {
    mDurationGroup->setEnabled(enabled);
    mChannelsGroup->setEnabled(enabled);
    mDestinationGroup->setEnabled(enabled);
}
