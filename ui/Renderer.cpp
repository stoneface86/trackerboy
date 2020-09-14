
#include "Renderer.hpp"


Renderer::Renderer(ModuleDocument &document, QObject *parent) :
    mDocument(document),
    mPb(audio::SR_44100),
    mSynth(44100.0f)
{
}


void Renderer::previewWaveform(trackerboy::Note note) {

}

void Renderer::previewInstrument(trackerboy::Note note) {

}

void Renderer::setPreviewNote(trackerboy::Note note) {

}

void Renderer::stopPreview() {

}
