
#pragma once

#include <QObject>

#include "audio.hpp"

#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/note.hpp"

#include "model/ModuleDocument.hpp"


//
// Renderer class handles pattern playback and instrument/waveform previews
//
class Renderer : public QObject {

public:
    Renderer(ModuleDocument &document, QObject *parent = nullptr);

    void previewWaveform(trackerboy::Note note);

    void previewInstrument(trackerboy::Note note);

    void setPreviewNote(trackerboy::Note note);

    void stopPreview();


signals:
    void playing(); // emitted when music starts playing
    void stopped(); // emitted when music stops playing via halt effect or by user action


private:

    ModuleDocument &mDocument;
    audio::PlaybackQueue mPb;
    trackerboy::Synth mSynth;


};
