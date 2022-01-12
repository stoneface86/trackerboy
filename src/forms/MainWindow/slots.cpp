
#include "forms/MainWindow.hpp"

#include "utils/connectutils.hpp"
#include "export/ExportWavDialog.hpp"
#include "forms/ModulePropertiesDialog.hpp"
#include "widgets/TableView.hpp"
#include "version.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QStringBuilder>
#include <QUndoView>
#include <QShortcut>
#include <QMenuBar>

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
    initSplitters();
}

void MainWindow::onMidiError() {
    if (isVisible()) {
        QMessageBox msgbox(this);
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("MIDI device error"));
        msgbox.setDetailedText(mMidi.lastError());
        settingsMessageBox(msgbox);
    }
}

Config::Categories MainWindow::applyConfig(Config const& config, Config::Categories categories, QString *problems) {

    Config::Categories flags = Config::CategoryNone;

    if (categories.testFlag(Config::CategoryGeneral)) {
        auto const& general = config.general();
        // options
        mPatternEditor->grid()->setShowFlats(general.hasOption(GeneralConfig::OptionShowFlats));
        mPatternModel->setPreviewEnable(general.hasOption(GeneralConfig::OptionShowPreviews));
        mPatternEditor->setRownoHex(general.hasOption(GeneralConfig::OptionRownoHex));
        mPatternModel->setCursorWrap(general.hasOption(GeneralConfig::OptionCursorWrap));
        mPatternModel->setCursorWrapPattern(general.hasOption(GeneralConfig::OptionCursorWrapPattern));
        mModuleFile.setAutoBackup(general.hasOption(GeneralConfig::OptionBackupCopy));

        // autosave
        mAutosave = general.hasAutosave();
        mAutosaveIntervalMs = general.autosaveInterval() * 1000;

        // page step
        mPatternEditor->setPageStep(general.pageStep());
    }


    if (categories.testFlag(Config::CategorySound)) {
        auto const& sound = config.sound();
        mStatusSamplerate->setText(tr("%1 Hz").arg(sound.samplerate()));

        mErrorSinceLastConfig = !mRenderer->setConfig(sound, mAudioEnumerator);
        if (mErrorSinceLastConfig) {
            flags |= Config::CategorySound;
            setPlayingStatus(PlayingStatusText::error);
            if (problems) {
                problems->append(tr("[Sound] The configured device could not be initialized. Playback is disabled.\n"));
            }
        } else {
            if (!mRenderer->isRunning()) {
                setPlayingStatus(PlayingStatusText::ready);
            }
        }
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto const& fonts = config.fonts();
        mPalette = config.palette();

        mPatternEditor->grid()->setFont(fonts[FontTable::PatternGrid]);
        
        mPatternEditor->setColors(mPalette);

        mPatternEditor->gridHeader()->setFont(fonts[FontTable::PatternGridHeader]);

        auto orderGrid = mSidebar->orderEditor()->grid();
        orderGrid->setFont(fonts[FontTable::OrderGrid]);
        orderGrid->setColors(mPalette);

        mSidebar->scope()->setColors(mPalette);
        if (mInstrumentEditor) {
            mInstrumentEditor->setColors(mPalette);
        }
        if (mWaveEditor) {
            mWaveEditor->setColors(mPalette);
        }
    }

    if (categories.testFlag(Config::CategoryKeyboard)) {
        mPianoInput = config.pianoInput();

        auto &shortcuts = config.shortcuts();
        #define setShortcut(shortcut, key) shortcut->setKey(shortcuts.get(ShortcutTable::key))
        setShortcut(mShortcutPrevInst, PrevInstrument);
        setShortcut(mShortcutNextInst, NextInstrument);
        setShortcut(mShortcutPrevPatt, PrevPattern);
        setShortcut(mShortcutNextPatt, NextPattern);
        setShortcut(mShortcutDecOct, DecOctave);
        setShortcut(mShortcutIncOct, IncOctave);
        setShortcut(mShortcutPlayStop, PlayStop);
        #undef setShortcut

        mInstruments->setShortcut(shortcuts.get(ShortcutTable::EditInstrument));
        mWaveforms->setShortcut(shortcuts.get(ShortcutTable::EditWaveform));

        configureActions(*menuBar(), shortcuts);

    }

    if (categories.testFlag(Config::CategoryMidi)) {
        auto const& midiConfig = config.midi();

        if (!midiConfig.isEnabled() || midiConfig.portIndex() == -1) {
            mMidi.close();
        } else {
            auto device = mMidiEnumerator.device(midiConfig.backendIndex(), midiConfig.portIndex());
            auto success = mMidi.open(device);
            if (!success) {
                flags |= Config::CategoryMidi;
                if (problems) {
                    problems->append(tr("[Midi] unable to open the device. Midi is disabled.\n"));
                }
                qCritical().noquote() << "[MIDI] Failed to initialize MIDI device:" << mMidi.lastError();
            }
        }
    }

    return flags;

}

void MainWindow::showAboutDialog() {

    QMessageBox::about(this, tr("About"),
        QStringLiteral(
R"bodystr(
<html><head/>
<body>
<h1>Trackerboy v%1</h1>
<h3>Gameboy/Gameboy Color music tracker</h3>
<h3>Copyright (C) 2019-2021 stoneface86</h3>
<br/>
<span>Repo - <a href="https://github.com/stoneface86/trackerboy">https://github.com/stoneface86/trackerboy</a></span>
<br/>
<span>Commit: %2</span>
<br/>
<p>This software is licensed under the MIT License.</p>
<hr/>
<h3>Libraries</h3>
<ul>
<li>miniaudio 0.10.42 (C) David Reid</li>
<li>RtMidi 4.0.0 (C) Gary P. Scavone</li>
</ul>
</body>
</html>
)bodystr").arg(QString::fromLatin1(VERSION_STR), QString::fromUtf8(GIT_SHA1)));

}

void MainWindow::showAudioDiag() {
    if (mAudioDiag == nullptr) {
        mAudioDiag = new AudioDiagDialog(*mRenderer, this);
    }

    mAudioDiag->show();
}

void MainWindow::showConfigDialog() {

    Config config;
    config.readSettings(mAudioEnumerator, mMidiEnumerator);

#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    ConfigDialog diag(config, mAudioEnumerator, mMidiEnumerator, this);
    connect(&diag, &ConfigDialog::applied, this,
        [this, &config](Config::Categories categories) {

            config.writeSettings(mAudioEnumerator, mMidiEnumerator);

            QString problems;
            auto flags = applyConfig(config, categories, &problems);
            if (flags) {
                auto sender_ = qobject_cast<ConfigDialog*>(sender());
                if (sender_) {
                    // problems with the config, keep the config dirty
                    sender_->unclean(flags);

                    // show the issue(s) to the user
                    QMessageBox msgbox(sender_);
                    msgbox.setIcon(QMessageBox::Critical);
                    msgbox.setText(tr("Problem(s) occurred when applying the config"));
                    msgbox.setDetailedText(problems);
                    msgbox.exec();
                }
            }
        });
    // optimize if showing the dialog takes longer than 250 ms
#ifdef QT_DEBUG
    qDebug() << "ConfigDialog creation took" << timer.elapsed() << "ms";
#endif
    diag.exec();

}

void MainWindow::showExportWavDialog() {
    ExportWavDialog dialog(*mModule, mModuleFile, mRenderer->samplerate(), this);
    dialog.exec();
}

void MainWindow::showTempoCalculator() {
    if (mTempoCalc == nullptr) {
        mTempoCalc = new TempoCalculator(*mSongModel, this);
    }
    mTempoCalc->show();
}

void MainWindow::showInstrumentEditor() {
    if (mInstrumentEditor == nullptr) {
        mInstrumentEditor = new InstrumentEditor(*mModule, *mInstrumentModel, *mWaveModel, mPianoInput, this);
        mInstrumentEditor->init();
        mInstrumentEditor->setColors(mPalette);
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
        mWaveEditor = new WaveEditor(*mModule, *mWaveModel, mPianoInput, this);
        mWaveEditor->init();
        mWaveEditor->setColors(mPalette);
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

void MainWindow::showHistory() {
    if (mHistoryDialog == nullptr) {
        mHistoryDialog = new PersistantDialog(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        auto layout = new QVBoxLayout;
        auto undoView = new QUndoView(mModule->undoGroup());
        layout->addWidget(undoView);
        mHistoryDialog->setLayout(layout);
        mHistoryDialog->setWindowTitle(tr("History"));
    } 
    mHistoryDialog->show();
    
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

void MainWindow::previousInstrument() {
    mInstruments->setSelectedItem(mInstruments->selectedItem() - 1);
}

void MainWindow::nextInstrument() {
    mInstruments->setSelectedItem(mInstruments->selectedItem() + 1);
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
