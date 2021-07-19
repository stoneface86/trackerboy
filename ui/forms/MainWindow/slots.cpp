
#include "forms/MainWindow.hpp"

#include "core/midi/MidiProber.hpp"
#include "core/samplerates.hpp"

#include <QFileDialog>

static const char* MODULE_FILE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");

// action slots

void MainWindow::onFileNew() {
    if (!maybeSave()) {
        return;
    }

    //mRenderer.forceStop();

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

    //mRenderer.forceStop();

    bool opened = mModuleFile.open(path, *mModule);

    if (!opened) {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Could not open module"));
        
        auto error = mModuleFile.lastError();
        switch (error) {
            case trackerboy::FormatError::duplicateId:
            case trackerboy::FormatError::invalid:
            case trackerboy::FormatError::tableDuplicateId:
            case trackerboy::FormatError::tableSizeBounds:
            case trackerboy::FormatError::unknownChannel:
                msgbox.setInformativeText(tr("The module is corrupted"));
                break;
            case trackerboy::FormatError::invalidSignature:
                msgbox.setInformativeText(tr("The file is not a trackerboy module"));
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
     auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        "",
        tr(MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    auto result = mModuleFile.save(path, *mModule);
    if (result) {
        // the document has a new name, update the window title
        updateWindowTitle();
    }
    return result;
}

void MainWindow::onSongOrderInsert() {
    mOrderModel->insert();
    updateOrderActions();
}

void MainWindow::onSongOrderRemove() {
    mOrderModel->remove();
    updateOrderActions();
}

void MainWindow::onSongOrderDuplicate() {
    mOrderModel->duplicate();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveUp() {
    mOrderModel->moveUp();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveDown() {
    mOrderModel->moveDown();
    updateOrderActions();
}

void MainWindow::onViewResetLayout() {
    // remove everything
    removeToolBar(mToolbarFile);
    removeToolBar(mToolbarEdit);
    removeToolBar(mToolbarTracker);
    removeToolBar(mToolbarInput);
    removeToolBar(mToolbarInstrument);

    initState();
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mConfig.sound();
        auto samplerate = SAMPLERATE_TABLE[sound.samplerateIndex];
        mStatusSamplerate.setText(tr("%1 Hz").arg(samplerate));

        // mErrorSinceLastConfig = !mRenderer.setConfig(sound);
        // if (mErrorSinceLastConfig) {
        //     setPlayingStatus(PlayingStatusText::error);
        //     if (isVisible()) {
        //         QMessageBox msgbox(this);
        //         msgbox.setIcon(QMessageBox::Critical);
        //         msgbox.setText(tr("Could not initialize device"));
        //         msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
        //         settingsMessageBox(msgbox);
        //     }
        // } else {
        //     if (!mRenderer.isRunning()) {
        //         setPlayingStatus(PlayingStatusText::ready);
        //     }
        // }
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto &appearance = mConfig.appearance();

        // see resources/stylesheet.qss
        QFile styleFile(QStringLiteral(":/stylesheet.qss"));
        styleFile.open(QFile::ReadOnly);

        QString stylesheet(styleFile.readAll());

        setStyleSheet(stylesheet.arg(
            appearance.colors[+Color::background].name(),
            appearance.colors[+Color::line].name(),
            appearance.colors[+Color::foreground].name(),
            appearance.colors[+Color::selection].name(),
            appearance.font.family(),
            QString::number(appearance.font.pointSize())
        ));

        //mPatternEditor.setColors(appearance.colors);
        OrderModel::setRowColor(appearance.colors[+Color::row]);
    }

    if (categories.testFlag(Config::CategoryKeyboard)) {
        mPianoInput = mConfig.keyboard().pianoInput;
    }

    if (categories.testFlag(Config::CategoryMidi)) {
        auto midiConfig = mConfig.midi();
        auto &prober = MidiProber::instance();
        if (!midiConfig.enabled || midiConfig.portIndex == -1) {
            mMidi.close();
        } else {
            auto success = mMidi.setDevice(prober.backend(), midiConfig.portIndex);
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
    // if (mAudioDiag == nullptr) {
    //     mAudioDiag = new AudioDiagDialog(mRenderer, this);
    // }

    // mAudioDiag->show();
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
    // auto dialog = new ExportWavDialog(mDocument, mConfig, this);
    // dialog->show();
    // dialog->exec();
    // delete dialog;
}

void MainWindow::onAudioStart() {
    // if (!mRenderer.isRunning()) {
    //     return;
    // }

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
    // if (mRenderer.isRunning()) {
    //     return; // sometimes it takes too long for this signal to get here
    // }

    if (!mErrorSinceLastConfig) {
        setPlayingStatus(PlayingStatusText::ready);
    }
}

void MainWindow::onFrameSync() {
    // this slot is called when the renderer has renderered a new frame
    // sync is a bit misleading here, as this slot is called when this frame
    // is in process of being bufferred. It is not the current frame being played out.

    // auto frame = mRenderer.currentFrame();

    // // check if the player position changed
    // if (frame.startedNewRow) {        
    //     // update tracker position
    //     mDocument.patternModel().setTrackerCursor(frame.row, frame.order);

    //     // update position status
    //     mStatusPos.setText(QStringLiteral("%1 / %2")
    //         .arg(frame.order, 2, 10, QChar('0'))
    //         .arg(frame.row, 2, 10, QChar('0')));
    // }

    // // check if the speed changed
    // if (mLastEngineFrame.speed != frame.speed) {
    //     auto speedF = trackerboy::speedToFloat(frame.speed);
    //     // update speed status
    //     mStatusSpeed.setText(tr("%1 FPR").arg(speedF, 0, 'f', 3));
    //     auto tempo = trackerboy::speedToTempo(speedF, mDocument.songModel().rowsPerBeat());
    //     mStatusTempo.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
    // }

    // constexpr auto FRAME_SKIP = 30;

    // if (mLastEngineFrame.time != frame.time) {
    //     // determine elapsed time
    //     if (mFrameSkip == 0) {

    //         auto framerate = mDocument.framerate();
    //         int elapsed = frame.time / framerate;
    //         int secs = elapsed;
    //         int mins = secs / 60;
    //         secs = secs % 60;

    //         QString str = QStringLiteral("%1:%2")
    //             .arg(mins, 2, 10, QChar('0'))
    //             .arg(secs, 2, 10, QChar('0'));
    //         mStatusElapsed.setText(str);


    //         mFrameSkip = FRAME_SKIP;
    //     } else {
    //         --mFrameSkip;
    //     }
    // }

    // mLastEngineFrame = frame;
}
