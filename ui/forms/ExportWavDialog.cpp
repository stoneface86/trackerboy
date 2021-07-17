
#include "core/samplerates.hpp"
#include "forms/ExportWavDialog.hpp"

#include "core/Config.hpp"
#include "core/Document.hpp"
#include "core/WavExporter.hpp"

#include <QFileDialog>
#include <QFileInfo>

ExportWavDialog::ExportWavDialog(Document &document, Config &config, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mDocument(document),
    mConfig(config),
    mLayout(),
    mDurationGroup(tr("Duration")),
    mDurationLayout(),
    mLoopRadio(tr("Play the song")),
    mLoopSpin(),
    mLoopLabel(tr("time(s)")),
    mTimeRadio(tr("Play for")),
    mTimeEdit(QStringLiteral("01:00")),
    mTimeLabel(tr("mm:ss")),
    mChannelGroup(tr("Channels")),
    mChannelLayout(),
    mChannelSelectLayout(),
    mChannelChecks(),
    mSeparateChannelsCheck(tr("Export each channel separately")),
    mDestinationGroup(tr("Destination")),
    mDestinationLayout(),
    mFilenameEdit(),
    mBrowseButton(tr("Browse")),
    mProgress(),
    mStatusLabel(),
    mButtons(),
    mExportButton(nullptr),
    mExporter(nullptr),
    mTimeEditDuration(60)
{
    setModal(true);
    setWindowTitle(tr("Export to WAV"));

    mDurationLayout.addWidget(&mLoopRadio, 0, 0);
    mDurationLayout.addWidget(&mLoopSpin, 0, 1);
    mDurationLayout.addWidget(&mLoopLabel, 0, 2);
    mDurationLayout.addWidget(&mTimeRadio, 1, 0);
    mDurationLayout.addWidget(&mTimeEdit, 1, 1);
    mDurationLayout.addWidget(&mTimeLabel, 1, 2);
    mDurationGroup.setLayout(&mDurationLayout);

    int ch = 1;
    for (auto &check : mChannelChecks) {
        check.setText(QString::number(ch));
        check.setChecked(true);
        mChannelSelectLayout.addWidget(&check);
        connect(&check, &QCheckBox::toggled, this,
            [this, &check](bool toggled) {
                if (toggled) {
                    for (auto &box : mChannelChecks) {
                        box.setEnabled(true);
                    }
                } else {
                    int checkCount = 0;
                    QCheckBox *lastChecked = nullptr;
                    for (auto &box : mChannelChecks) {
                        if (box.isChecked()) {
                            ++checkCount;
                            lastChecked = &box;
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
    mChannelSelectLayout.addStretch();
    mChannelLayout.addLayout(&mChannelSelectLayout);
    mChannelLayout.addWidget(&mSeparateChannelsCheck);
    mChannelGroup.setLayout(&mChannelLayout);

    mDestinationLayout.addWidget(&mFilenameEdit, 1);
    mDestinationLayout.addWidget(&mBrowseButton);
    mDestinationGroup.setLayout(&mDestinationLayout);

    mLayout.addWidget(&mDurationGroup);
    mLayout.addWidget(&mChannelGroup);
    mLayout.addWidget(&mDestinationGroup);
    mLayout.addWidget(&mProgress);
    mLayout.addWidget(&mStatusLabel);
    mLayout.addWidget(&mButtons);
    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(&mLayout);

    mLoopRadio.setChecked(true);
    mLoopSpin.setRange(1, 100);
    mTimeEdit.setInputMask(QStringLiteral("99:99"));
    mTimeEdit.setMaxLength(5);
    mProgress.setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    
    // disable this group since individual channel selection is not implemented at this time
    mChannelGroup.setEnabled(false);

    mExportButton = mButtons.addButton(tr("Export"), QDialogButtonBox::AcceptRole);
    mExportButton->setEnabled(false);
    mButtons.addButton(QDialogButtonBox::Cancel);

    connect(&mButtons, &QDialogButtonBox::accepted, this, &ExportWavDialog::accept);
    connect(&mButtons, &QDialogButtonBox::rejected, this, &ExportWavDialog::reject);

    connect(&mLoopSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this]() {
            mLoopRadio.setChecked(true);
        });
    
    connect(&mTimeEdit, &QLineEdit::textEdited, this,
        [this](QString const& text) {
            mTimeRadio.setChecked(true);

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
            mTimeEdit.setText(QStringLiteral("%1:%2")
                        .arg(mins, 2, 10, QChar('0'))
                        .arg(secs, 2, 10, QChar('0')));
        });

    connect(&mFilenameEdit, &QLineEdit::textChanged, this,
        [this](QString const& str) {
            mExportButton->setEnabled(!str.isEmpty());
        });

    connect(&mBrowseButton, &QPushButton::clicked, this,
        [this]() {
            auto filename = QFileDialog::getSaveFileName(
                this,
                tr("Select destination"),
                mFilenameEdit.text(),
                tr("WAV files (*.wav)")
            );

            if (filename.isEmpty()) {
                return;
            }

            mFilenameEdit.setText(filename);
        });
    
    QString destinationName;
    if (mDocument.hasFile()) {
        // initialize the destination file with the module filename
        QFileInfo info(mDocument.filepath());
        destinationName = info.dir().filePath(info.completeBaseName());
    } else {
        destinationName = QDir::home().filePath(mDocument.name());
    }
    mFilenameEdit.setText(QStringLiteral("%1.wav").arg(destinationName));
    
}

void ExportWavDialog::accept() {
    // begin the export
    bool isExporting = false;
    if (mExporter && mExporter->isRunning()) {
        isExporting = true;
    }

    if (!isExporting) {
        if (mExporter == nullptr) {
            mExporter = new WavExporter(mDocument.mod(), SAMPLERATE_TABLE[mConfig.sound().samplerateIndex], this);
            connect(mExporter, &WavExporter::progressMax, &mProgress, &QProgressBar::setMaximum);
            connect(mExporter, &WavExporter::progress, &mProgress, &QProgressBar::setValue);
            connect(mExporter, &WavExporter::finished, this,
                [this]() {
                    if (mExporter->failed()) {
                        mStatusLabel.setText(tr("Export failed"));
                    } else {
                        mProgress.setValue(mProgress.maximum());
                        mStatusLabel.setText(tr("Export complete"));
                    }
                    mExportButton->setEnabled(true);
                    setGroupsEnabled(true);
                });
        }

        if (mLoopRadio.isChecked()) {
            mExporter->setDuration(mLoopSpin.value());
        } else {
            mExporter->setDuration(std::chrono::seconds(mTimeEditDuration));
        }

        mExporter->setDestination(mFilenameEdit.text());

        mStatusLabel.setText(tr("Exporting..."));
        mProgress.setValue(0);
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
    mDurationGroup.setEnabled(enabled);
    mDestinationGroup.setEnabled(enabled);
}
