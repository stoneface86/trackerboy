
#include "forms/MainWindow.hpp"

#include "core/midi/MidiProber.hpp"
#include "forms/ExportWavDialog.hpp"

#include <QFileDialog>

static const char* MODULE_FILE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");

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
        tr(MODULE_FILE_FILTER)
    );

    if (path.isEmpty()) {
        return;
    }

    mRenderer->forceStop();

    bool opened = mModuleFile.open(path, *mModule);

    if (!opened) {
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
        commitModels();
        return mModuleFile.save(*mModule);
    } else {
        return onFileSaveAs();
    }
}

bool MainWindow::onFileSaveAs() {
     auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        "",
        tr(MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    commitModels();
    auto result = mModuleFile.save(path, *mModule);
    if (result) {
        // the document has a new name, update the window title
        updateWindowTitle();
    }
    return result;
}

void MainWindow::onSongOrderInsert() {
    //mOrderModel->insert();
    updateOrderActions();
}

void MainWindow::onSongOrderRemove() {
    //mOrderModel->remove();
    updateOrderActions();
}

void MainWindow::onSongOrderDuplicate() {
    //mOrderModel->duplicate();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveUp() {
    //mOrderModel->moveUp();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveDown() {
    //mOrderModel->moveDown();
    updateOrderActions();
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

        auto font = appearance.font();
        
        mPatternEditor->setFont(font);
        mPatternEditor->setColors(pal);

        auto orderGrid = mSidebar->orderEditor()->grid();
        orderGrid->setFont(font);
        orderGrid->setColors(pal);
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

    mConfig.writeSettings();
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
    if (mConfigDialog == nullptr) {
        mConfigDialog = new ConfigDialog(mConfig, this);
        mConfigDialog->resetControls();

        connect(mConfigDialog, &ConfigDialog::applied, this, &MainWindow::onConfigApplied);
    }

    mConfigDialog->show();
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

void MainWindow::previousInstrument() {
    // TODO
}

void MainWindow::nextInstrument() {
    // TODO
}

void MainWindow::previousPattern() {
    // TODO
}

void MainWindow::nextPattern() {
    // TODO
}

void MainWindow::increaseOctave() {
    // TODO
}

void MainWindow::decreaseOctave() {
    // TODO
}

void MainWindow::playOrStop() {
    if (mRenderer->isRunning()) {
        onTrackerStop();
    } else {
        onTrackerPlay();
    }
}
