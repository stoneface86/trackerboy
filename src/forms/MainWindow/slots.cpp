
#include "forms/MainWindow.hpp"

#include "core/midi/MidiProber.hpp"
#include "core/misc/connectutils.hpp"
#include "forms/ExportWavDialog.hpp"
#include "forms/ModulePropertiesDialog.hpp"
#include "widgets/docks/TableDock.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QStringBuilder>

#define TU MainWindowTU
namespace TU {

static const char* MODULE_FILE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");

}

// action slots

void MainWindow::onFileNew() {
    if (!maybeSave()) {
        return;
    }

    mRenderer->forceStop();

    mModule->clear();

    mModuleFile.setName(mUntitledString);
    updateWindowTitle();

}

void MainWindow::onFileOpen() {
    if (!maybeSave()) {
        return;
    }
    
    auto path = QFileDialog::getOpenFileName(
        this,
        tr("Open module"),
        "",
        tr(TU::MODULE_FILE_FILTER)
    );

    if (path.isEmpty()) {
        return;
    }

    openFile(path);

}

void MainWindow::openFile(QString const& path) {
    mRenderer->forceStop();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool opened = mModuleFile.open(path, *mModule);
    QApplication::restoreOverrideCursor();

    if (opened) {
        pushRecentFile(path);
    } else {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Could not open module"));

        auto error = mModuleFile.lastError();
        switch (error) {
            case trackerboy::FormatError::invalidSignature:
                msgbox.setInformativeText(tr("The file is not a trackerboy module"));
                break;
            case trackerboy::FormatError::invalidRevision:
                msgbox.setInformativeText(tr("The module is from a newer version of Trackerboy"));
                break;
            case trackerboy::FormatError::cannotUpgrade:
                msgbox.setInformativeText(tr("Failed to upgrade the module"));
                break;
            case trackerboy::FormatError::duplicateId:
            case trackerboy::FormatError::invalid:
            case trackerboy::FormatError::unknownChannel:
                msgbox.setInformativeText(tr("The module is corrupted"));
                break;
            default:
                msgbox.setInformativeText(tr("The file could not be read"));
                break;
        }


        msgbox.exec();
        mModuleFile.setName(mUntitledString);

    }

    // update window title with document name
    updateWindowTitle();
}

bool MainWindow::onFileSave() {
    if (mModuleFile.hasFile()) {
        return mModuleFile.save(*mModule);
    } else {
        return onFileSaveAs();
    }
}

bool MainWindow::onFileSaveAs() {

    QString curPath;
    if (mModuleFile.hasFile()) {
        curPath = mModuleFile.filepath();
    } else {
        curPath = mModuleFile.name() % QStringLiteral(".tbm");
    }

    auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        curPath,
        tr(TU::MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    auto result = mModuleFile.save(path, *mModule);
    if (result) {
        pushRecentFile(path);
        // the document has a new name, update the window title
        updateWindowTitle();
    } else {
        QMessageBox::critical(
            this,
            tr("Save failed"),
            tr("The module could not be written")
        );
    }
    return result;
}

void MainWindow::onFileRecent() {

    // sender should always be a QAction in the mRecentFilesActions array,
    // but check anyways
    auto action = qobject_cast<QAction*>(sender());
    if (action) {
        // the filename to open is stored in action's status tip
        auto path = action->statusTip();
        if (!path.isEmpty() && maybeSave()) {
            openFile(path);
        }
    }

}

void MainWindow::onModuleComments() {
    if (mCommentsDialog == nullptr) {
        mCommentsDialog = new CommentsDialog(*mModule, this);
    }
    mCommentsDialog->show();
}

void MainWindow::onModuleModuleProperties() {
    mRenderer->forceStop();

    ModulePropertiesDialog diag(*mModule, *mSongListModel, this);
    diag.show();
    int code = diag.exec();

    if (code == ModulePropertiesDialog::AcceptedSystemChange) {
        mRenderer->updateFramerate();
    }
}

bool MainWindow::checkAndStepOut() {
    if (mRenderer->isStepping()) {
        mRenderer->stepOut();
        return true;
    } else {
        return false;
    }
}

void MainWindow::onTrackerPlay() {
    if (!checkAndStepOut()) {
        mRenderer->play(mPatternModel->cursorPattern(), 0, false);
    }
}

void MainWindow::onTrackerPlayAtStart() {
    mRenderer->play(0, 0, false);
}

void MainWindow::onTrackerPlayFromCursor() {
    if (!checkAndStepOut()) {
        mRenderer->play(mPatternModel->cursorPattern(), mPatternModel->cursorRow(), false);
    }
    
}

void MainWindow::onTrackerStep() {
    if (mRenderer->isStepping()) {
        mRenderer->stepNextFrame();
    } else {
        mRenderer->play(mPatternModel->cursorPattern(), mPatternModel->cursorRow(), true);
    }
}

void MainWindow::onTrackerStop() {
    mRenderer->stopMusic();
}

void MainWindow::onTrackerSolo() {
    mPatternEditor->gridHeader()->soloTrack(mPatternModel->cursorTrack());
}

void MainWindow::onTrackerToggleOutput() {
    mPatternEditor->gridHeader()->toggleTrack(mPatternModel->cursorTrack());
}

void MainWindow::onTrackerKill() {
    mRenderer->forceStop();
}


void MainWindow::onViewResetLayout() {
    // remove everything
    removeToolBar(mToolbarFile);
    removeToolBar(mToolbarEdit);
    removeToolBar(mToolbarTracker);
    removeToolBar(mToolbarInput);

    initState();
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    applyConfig(categories);
    mConfig.writeSettings();
}

void MainWindow::applyConfig(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto const& sound = mConfig.sound();
        mStatusSamplerate->setText(tr("%1 Hz").arg(sound.samplerate()));

        mErrorSinceLastConfig = !mRenderer->setConfig(sound);
        if (mErrorSinceLastConfig) {
            setPlayingStatus(PlayingStatusText::error);
            if (isVisible()) {
                QMessageBox msgbox(this);
                msgbox.setIcon(QMessageBox::Critical);
                msgbox.setText(tr("Could not initialize device"));
                msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
                settingsMessageBox(msgbox);
            }
        } else {
            if (!mRenderer->isRunning()) {
                setPlayingStatus(PlayingStatusText::ready);
            }
        }
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto const& appearance = mConfig.appearance();
        auto const& pal = mConfig.palette();

        mPatternEditor->grid()->setFont(appearance.patternGridFont());
        mPatternEditor->grid()->setShowFlats(appearance.showFlats());
        mPatternModel->setPreviewEnable(appearance.showPreviews());
        mPatternEditor->setColors(pal);

        mPatternEditor->gridHeader()->setFont(appearance.patternGridHeaderFont());

        auto orderGrid = mSidebar->orderEditor()->grid();
        orderGrid->setFont(appearance.orderGridFont());
        orderGrid->setColors(pal);

        mSidebar->scope()->setColors(pal);
        if (mInstrumentEditor) {
            mInstrumentEditor->setColors(pal);
        }
        if (mWaveEditor) {
            mWaveEditor->setColors(pal);
        }
    }

    if (categories.testFlag(Config::CategoryKeyboard)) {
        //mPianoInput = mConfig.keyboard().pianoInput;
    }

    if (categories.testFlag(Config::CategoryMidi)) {
        auto const& midiConfig = mConfig.midi();
        auto &prober = MidiProber::instance();
        if (!midiConfig.isEnabled() || midiConfig.portIndex() == -1) {
            mMidi.close();
        } else {
            auto success = mMidi.setDevice(prober.backend(), midiConfig.portIndex());
            if (!success) {
                disableMidi(false);
            }
        }
    }

}

void MainWindow::showAboutDialog() {
    if (mAboutDialog == nullptr) {
        mAboutDialog = new AboutDialog(this);
    }
    mAboutDialog->show();
}

void MainWindow::showAudioDiag() {
    if (mAudioDiag == nullptr) {
        mAudioDiag = new AudioDiagDialog(*mRenderer, this);
    }

    mAudioDiag->show();
}

void MainWindow::showConfigDialog() {
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    ConfigDialog diag(mConfig, this);
    lazyconnect(&diag, applied, this, onConfigApplied);
#ifdef QT_DEBUG
    qDebug() << "ConfigDialog creation took" << timer.elapsed() << "ms";
#endif
    diag.exec();
}

void MainWindow::showExportWavDialog() {
    auto dialog = new ExportWavDialog(*mModule, mModuleFile, mConfig.sound().samplerate(), this);
    dialog->show();
    dialog->exec();
    delete dialog;
}

void MainWindow::showTempoCalculator() {
    if (mTempoCalc == nullptr) {
        mTempoCalc = new TempoCalculator(*mSongModel, this);
    }
    mTempoCalc->show();
}

void MainWindow::showInstrumentEditor() {
    if (mInstrumentEditor == nullptr) {
        mInstrumentEditor = new InstrumentEditor(*mModule, *mInstrumentModel, *mWaveModel, mConfig.pianoInput(), this);
        mInstrumentEditor->init();
        mInstrumentEditor->setColors(mConfig.palette());
        auto piano = mInstrumentEditor->piano();
        connect(piano, &PianoWidget::keyDown, this,
            [this](int note) {
                auto item = mInstrumentEditor->currentItem();
                if (item != -1) {
                    mRenderer->instrumentPreview(note, -1, mInstrumentModel->id(item));
                }
            });

        lazyconnect(piano, keyChange, mRenderer, setPreviewNote);
        lazyconnect(piano, keyUp, mRenderer, stopPreview);
        lazyconnect(mInstrumentEditor, openWaveEditor, this, editWaveform);
    }

    mInstrumentEditor->show();
}

void MainWindow::showWaveEditor() {
    if (mWaveEditor == nullptr) {
        mWaveEditor = new WaveEditor(*mModule, *mWaveModel, mConfig.pianoInput(), this);
        mWaveEditor->init();
        mWaveEditor->setColors(mConfig.palette());
        auto piano = mWaveEditor->piano();
        connect(piano, &PianoWidget::keyDown, this,
            [this](int note) {
                auto item = mWaveEditor->currentItem();
                if (item != -1) {
                    mRenderer->waveformPreview(note, mWaveModel->id(item));
                }
            });
        lazyconnect(piano, keyChange, mRenderer, setPreviewNote);
        lazyconnect(piano, keyUp, mRenderer, stopPreview);
    }
    mWaveEditor->show();
}

void MainWindow::onAudioStart() {
    if (!mRenderer->isRunning()) {
        return;
    }

    mLastEngineFrame = {};
    mFrameSkip = 0;
    setPlayingStatus(PlayingStatusText::playing);
}

void MainWindow::onAudioError() {
    setPlayingStatus(PlayingStatusText::error);
    if (!mErrorSinceLastConfig) {
        mErrorSinceLastConfig = true;
        QMessageBox msgbox(this);
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Audio error"));
        msgbox.setInformativeText(tr(
            "A device error has occurred during playback.\n\n" \
            "Playback is disabled until a new device is configured in the settings."
        ));
        settingsMessageBox(msgbox);

    }
    onAudioStop();
}

void MainWindow::onAudioStop() {
    if (mRenderer->isRunning()) {
        return; // sometimes it takes too long for this signal to get here
    }

    mPatternModel->setPlaying(false);

    if (!mErrorSinceLastConfig) {
        setPlayingStatus(PlayingStatusText::ready);
    }
}

void MainWindow::onFrameSync() {
    // this slot is called when the renderer has renderered a new frame
    // sync is a bit misleading here, as this slot is called when this frame
    // is in process of being bufferred. It is not the current frame being played out.

    auto frame = mRenderer->currentFrame();

    // check if the player position changed
    if (frame.startedNewRow) {        
        // update tracker position
        mPatternModel->setTrackerCursor(frame.row, frame.order);

        // update position status
        mStatusPos->setText(QStringLiteral("%1 / %2")
            .arg(frame.order, 2, 10, QChar('0'))
            .arg(frame.row, 2, 10, QChar('0')));
    }

    // check if the speed changed
    if (mLastEngineFrame.speed != frame.speed) {
        auto speedF = trackerboy::speedToFloat(frame.speed);
        // update speed status
        mStatusSpeed->setSpeed(speedF);
        auto tempo = trackerboy::speedToTempo(speedF,  mSongModel->rowsPerBeat());
        mStatusTempo->setTempo(tempo);
    }

    constexpr auto FRAME_SKIP = 30;

    if (mLastEngineFrame.time != frame.time) {
        // determine elapsed time
        if (mFrameSkip == 0) {

            //auto framerate = mDocument.framerate();
            int elapsed = frame.time / 60;
            int secs = elapsed;
            int mins = secs / 60;
            secs = secs % 60;

            QString str = QStringLiteral("%1:%2")
                .arg(mins, 2, 10, QChar('0'))
                .arg(secs, 2, 10, QChar('0'));
            mStatusElapsed->setText(str);


            mFrameSkip = FRAME_SKIP;
        } else {
            --mFrameSkip;
        }
    }

    mLastEngineFrame = frame;
}

namespace TU {

void incrementSelectedItemInTableDock(QDockWidget *dock, int amount) {
    auto table = qobject_cast<TableDock*>(dock->widget());
    if (table) {
        table->setSelectedItem(table->selectedItem() + amount);
    }
}

}

void MainWindow::previousInstrument() {
    TU::incrementSelectedItemInTableDock(mDockInstruments, -1);
}

void MainWindow::nextInstrument() {
    TU::incrementSelectedItemInTableDock(mDockInstruments, 1);
}

void MainWindow::previousPattern() {
    mPatternModel->setCursorPattern(mPatternModel->cursorPattern() - 1);
}

void MainWindow::nextPattern() {
    mPatternModel->setCursorPattern(mPatternModel->cursorPattern() + 1);
}

void MainWindow::increaseOctave() {
    mOctaveSpin->setValue(mOctaveSpin->value() + 1);
}

void MainWindow::decreaseOctave() {
    mOctaveSpin->setValue(mOctaveSpin->value() - 1);
}

void MainWindow::playOrStop() {
    if (mRenderer->isRunning()) {
        onTrackerStop();
    } else {
        onTrackerPlay();
    }
}

void MainWindow::onPatternCountChanged(int count) {
    bool canInsert = count < (int)trackerboy::MAX_PATTERNS;
    mActionOrderInsert->setEnabled(canInsert);
    mActionOrderRemove->setEnabled(count > 1);
    mActionOrderDuplicate->setEnabled(canInsert);
}

void MainWindow::onPatternCursorChanged(int pattern) {
    mActionOrderMoveUp->setEnabled(pattern > 0);
    mActionOrderMoveDown->setEnabled(pattern != mPatternModel->patterns() - 1);
}

void MainWindow::editInstrument(int item) {
    showInstrumentEditor();
    mInstrumentEditor->openItem(item);
}

void MainWindow::editWaveform(int item) {
    showWaveEditor();
    mWaveEditor->openItem(item);
}

#undef TU
