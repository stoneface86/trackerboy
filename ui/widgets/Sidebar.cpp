
#include "widgets/Sidebar.hpp"
#include "core/model/ModuleDocument.hpp"
#include "misc/IconManager.hpp"
#include "misc/utils.hpp"

#include <QHeaderView>

#include <cmath>

Sidebar::Sidebar(ModuleDocument &document, QWidget *parent) :
    QWidget(parent),
    mDocument(document),
    mIgnoreSelect(false),
    mLayout(),
    mScope(),
    mOrderGroup(tr("Song order")),
    mOrderLayout(),
    mOrderButtonLayout(),
    mOrderToolbar(),
    mSetSpin(),
    mSetButton(tr("Set")),
    mOrderView(),
    mSongGroup(tr("Song settings")),
    mSongLayout(),
    mRowsPerBeatLabel(tr("Rows/beat")),
    mRowsPerBeatSpin(),
    mRowsPerMeasureLabel(tr("Rows/measure")),
    mRowsPerMeasureSpin(),
    mSpeedLabel(tr("Speed")),
    mSpeedSpin(),
    mSpeedActual(),
    mTempoLabel(tr("Tempo")),
    mTempoSpin(),
    mTempoActual(),
    mRowsLabel(tr("Rows")),
    mRowsSpin(),
    mPatternsLabel(tr("Patterns")),
    mPatternsSpin()
{

    mOrderButtonLayout.addWidget(&mOrderToolbar, 1);
    mOrderButtonLayout.addWidget(&mSetSpin);
    mOrderButtonLayout.addWidget(&mSetButton);

    mOrderLayout.addLayout(&mOrderButtonLayout);
    mOrderLayout.addWidget(&mOrderView, 1);
    mOrderGroup.setLayout(&mOrderLayout);

    mSongLayout.addWidget(&mRowsPerBeatLabel, 0, 0);
    mSongLayout.addWidget(&mRowsPerBeatSpin, 0, 1);
    mSongLayout.addWidget(&mRowsPerMeasureLabel, 1, 0);
    mSongLayout.addWidget(&mRowsPerMeasureSpin, 1, 1);
    mSongLayout.addWidget(&mSpeedLabel, 2, 0);
    mSongLayout.addWidget(&mSpeedSpin, 2, 1);
    mSongLayout.addWidget(&mSpeedActual, 2, 2);
    mSongLayout.addWidget(&mTempoLabel, 3, 0);
    mSongLayout.addWidget(&mTempoSpin, 3, 1);
    mSongLayout.addWidget(&mTempoActual, 3, 2);
    mSongLayout.addWidget(&mRowsLabel, 4, 0);
    mSongLayout.addWidget(&mRowsSpin, 4, 1);
    mSongLayout.addWidget(&mPatternsLabel, 5, 0);
    mSongLayout.addWidget(&mPatternsSpin, 5, 1);
    mSongGroup.setLayout(&mSongLayout);

    mLayout.addWidget(&mScope);
    mLayout.addWidget(&mSongGroup);
    mLayout.addWidget(&mOrderGroup, 1);
    setLayout(&mLayout);

    setupAction(mActionIncrement, "Increment selection", "Increments all selected cells by 1", Icons::increment);
    setupAction(mActionDecrement, "Decrement selection", "Decrements all selected cells by 1", Icons::decrement);
    mOrderToolbar.addAction(&mActionIncrement);
    mOrderToolbar.addAction(&mActionDecrement);
    mOrderToolbar.setIconSize(QSize(16, 16));


    mOrderView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mOrderView, &QTableView::customContextMenuRequested, this,
        [this](QPoint const& pos) {
            QPoint mapped = mOrderView.viewport()->mapToGlobal(pos);
            emit orderMenuRequested(mapped);
        });
    mOrderView.setTabKeyNavigation(false);
    auto headerView = mOrderView.horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    auto verticalHeader = mOrderView.verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    mRowsPerBeatSpin.setRange(1, 255);
    mRowsPerMeasureSpin.setRange(1, 255);
    mSpeedSpin.setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin.setDisplayIntegerBase(16);
    mSpeedSpin.setPrefix("0x");
    mTempoSpin.setRange(1, 10000);
    mTempoSpin.setValue(150);
    mTempoSpin.setSuffix(" BPM");
    mPatternsSpin.setRange(1, trackerboy::MAX_PATTERNS);
    mPatternsSpin.setRange(1, 256);

    mSpeedActual.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mTempoActual.setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mSetButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // buddies
    mRowsPerBeatLabel.setBuddy(&mRowsPerBeatSpin);
    mRowsPerMeasureLabel.setBuddy(&mRowsPerMeasureSpin);
    mSpeedLabel.setBuddy(&mSpeedSpin);
    mTempoLabel.setBuddy(&mTempoSpin);
    mRowsLabel.setBuddy(&mRowsSpin);

    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this]() {
            // human-readable speed value
            float speed = trackerboy::speedToFloat((trackerboy::Speed)mSpeedSpin.value());
            float tempo = calcActualTempo(speed);
            setTempoLabel(tempo);
            mSpeedLock = true;
            mTempoSpin.setValue((int)roundf(tempo));
            mSpeedLock = false;
        });
    connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            auto speed = trackerboy::speedToFloat((trackerboy::Speed)value);
            auto tempo = calcActualTempo(speed);
            setTempoLabel(tempo);
            mSpeedActual.setText(tr("%1 FPR").arg(speed, 0, 'f', 3));
            
            // determine the actual tempo from the speed
            if (!mSpeedLock) {
                mSpeedLock = true;
                mTempoSpin.setValue((int)roundf(tempo));
                mSpeedLock = false;
            }

        });
    connect(&mTempoSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            if (!mSpeedLock) {
                mSpeedLock = true;
                // convert tempo to speed and set in the speed spin
                // speed = (framerate * 60) / (tempo * rpb)
                float speed = (mDocument.mod().framerate() * 60.0f) / (value * mRowsPerBeatSpin.value());
                mSpeedSpin.setValue((int)roundf(speed * (1 << trackerboy::SPEED_FRACTION_BITS)));
                mSpeedLock = false;
            }
        });
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerBeatSpin, &QSpinBox::setMaximum);

    connect(&mActionIncrement, &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().incrementSelection(mOrderView.selectionModel()->selection());
        });

    connect(&mActionDecrement, &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().decrementSelection(mOrderView.selectionModel()->selection());
        });

    connect(&mSetButton, &QPushButton::clicked, this,
        [this]() {
            mDocument.orderModel().setSelection(
                mOrderView.selectionModel()->selection(),
                static_cast<uint8_t>(mSetSpin.value())
            );
        });

    auto &orderModel = document.orderModel();
    mOrderView.setModel(&orderModel);
    connect(&orderModel, &OrderModel::currentIndexChanged, this, &Sidebar::currentIndexChanged);
    auto selectionModel = mOrderView.selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &Sidebar::currentChanged);
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &Sidebar::selectionChanged);
    selectionModel->select(orderModel.currentIndex(), QItemSelectionModel::Select);

    mPatternsSpin.setValue(orderModel.rowCount());
    connect(&orderModel, &OrderModel::rowsInserted, this, &Sidebar::updatePatternsSpin);
    connect(&orderModel, &OrderModel::rowsRemoved, this, &Sidebar::updatePatternsSpin);
    connect(&mPatternsSpin, qOverload<int>(&QSpinBox::valueChanged), &orderModel, &OrderModel::setPatternCount);

    auto &songModel = document.songModel();
    mRowsPerBeatSpin.setValue(songModel.rowsPerBeat());
    mRowsPerMeasureSpin.setValue(songModel.rowsPerMeasure());
    mSpeedSpin.setValue(songModel.speed());
    mRowsSpin.setValue(songModel.patternSize());
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerBeat);
    connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerMeasure);
    connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setSpeed);
    connect(&mRowsSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setPatternSize);

}

AudioScope& Sidebar::scope() {
    return mScope;
}

void Sidebar::currentIndexChanged(QModelIndex const& index) {
    if (!mIgnoreSelect) {
        mOrderView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
}

void Sidebar::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);

    mIgnoreSelect = true;
    auto &orderModel = mDocument.orderModel();
    auto &patternModel = mDocument.patternModel();
    if (patternModel.isPlaying() && patternModel.isFollowing()) {
        if (orderModel.currentPattern() != current.row()) {
            // jump to this pattern instead of selecting it
            emit patternJumpRequested(current.row());
        }
        orderModel.selectTrack(current.column());
    } else {
        orderModel.select(current.row(), current.column());
    }
    mIgnoreSelect = false;
    
}

void Sidebar::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    // this slot is just for preventing the user from deselecting
    auto model = mOrderView.selectionModel();
    if (!model->hasSelection()) {
        // user deselected everything, force selection of the current index
        model->select(mOrderView.currentIndex(), QItemSelectionModel::Select);
    }
    
}

void Sidebar::setTempoLabel(float tempo) {
    mTempoActual.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
}

float Sidebar::calcActualTempo(float speed) {
    // actual tempo value
    //tempo = (framerate * 60) / (speed * rpb)
    //convert fixed point to floating point
    return (mDocument.mod().framerate() * 60.0f) / (speed * mRowsPerBeatSpin.value());
}

void Sidebar::updatePatternsSpin() {
    mPatternsSpin.setValue(mDocument.orderModel().rowCount());
}
