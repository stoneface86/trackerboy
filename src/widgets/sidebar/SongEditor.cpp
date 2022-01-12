
#include "widgets/sidebar/SongEditor.hpp"
#include "utils/connectutils.hpp"
#include "utils/string.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

SongEditor::SongEditor(SongModel &model, QWidget *parent) :
    QWidget(parent),
    mSongModel(model),
    mRowsPerBeatSpin(new QSpinBox),
    mRowsPerMeasureSpin(new QSpinBox),
    mSpeedSpin(new CustomSpinBox),
    mSpeedLabel(new QLabel),
    mTempoLabel(new QLabel),
    mPatternSizeButton(new QPushButton)
{

    auto layout = new QGridLayout;
    auto label = new QLabel(tr("Rows/beat"));
    label->setBuddy(mRowsPerBeatSpin);
    layout->addWidget(label, 0, 0);
    layout->addWidget(mRowsPerBeatSpin, 0, 1);
    //
    label = new QLabel(tr("Rows/measure"));
    label->setBuddy(mRowsPerMeasureSpin);
    layout->addWidget(label, 1, 0);
    layout->addWidget(mRowsPerMeasureSpin, 1, 1);
    //
    label = new QLabel(tr("Rows"));
    label->setBuddy(mPatternSizeButton);
    layout->addWidget(label, 2, 0);
    layout->addWidget(mPatternSizeButton, 2, 1);
    //
    label = new QLabel(tr("Speed"));
    label->setBuddy(mSpeedSpin);
    layout->addWidget(label, 3, 0);
    layout->addWidget(mSpeedSpin, 3, 1);
    //
    layout->addWidget(mSpeedLabel, 4, 0);
    layout->addWidget(mTempoLabel, 4, 1);

    setLayout(layout);

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mSpeedSpin->setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin->setDisplayIntegerBase(16);
    mSpeedSpin->setPrefix("0x");

    mSpeedLabel->setAlignment(Qt::AlignCenter);
    mTempoLabel->setAlignment(Qt::AlignCenter);

    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerBeatSpin, &QSpinBox::setMaximum);

    connect(&model, &SongModel::rowsPerBeatChanged, this,
        [this](int rpb) {
            QSignalBlocker blocker(mRowsPerBeatSpin);
            mRowsPerBeatSpin->setValue(rpb);
        });

    connect(&model, &SongModel::rowsPerMeasureChanged, this,
        [this](int rpm) {
            QSignalBlocker blocker(mRowsPerMeasureSpin);
            mRowsPerMeasureSpin->setValue(rpm);
        });

    connect(&model, &SongModel::speedChanged, this,
        [this](int speed) {
            QSignalBlocker blocker(mSpeedSpin);
            mSpeedSpin->setValue(speed);

            auto speedFloat = trackerboy::speedToFloat((trackerboy::Speed)speed);
            mSpeedLabel->setText(speedToString(speedFloat));
        });

    connect(&model, &SongModel::tempoChanged, this,
        [this](float tempo) {
            mTempoLabel->setText(tempoToString(tempo));
        });

    connect(&model, &SongModel::patternSizeChanged, this,
        [this](int rows) {
            mPatternSizeButton->setText(QString::number(rows));
        });

    model.reload();
    
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &model, &SongModel::setRowsPerBeat);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &model, &SongModel::setRowsPerMeasure);
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &model, &SongModel::setSpeed);

    //
    // a dialog is used for changing the pattern size instead of just a spin box. This is
    // because changing the pattern size is a destructive operation with no undo (Decreasing
    // the size results in existing data being truncated).
    //

    connect(mPatternSizeButton, &QPushButton::clicked, this,
        [this]() {
            QDialog diag(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
            diag.setWindowTitle(tr("Change pattern size"));

            QVBoxLayout layout;
                QLabel label(tr("Pattern size:"));
                QSpinBox sizeSpin;
                QLabel warning(tr("Warning: this action cannot be undone."));
                QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
            layout.addWidget(&label);
            layout.addWidget(&sizeSpin);
            layout.addWidget(&warning);
            layout.addWidget(&buttons);
            layout.setSizeConstraint(QLayout::SetFixedSize);
            diag.setLayout(&layout);

            auto const originalSize = mSongModel.patternSize();
            sizeSpin.setValue(originalSize);
            sizeSpin.setRange(1, 256);
            lazyconnect(&buttons, accepted, &diag, accept);
            lazyconnect(&buttons, rejected, &diag, reject);

            if (diag.exec() == QDialog::Accepted) {
                mSongModel.setPatternSize(sizeSpin.value());
            }
        });

}
