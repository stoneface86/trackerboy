
#include "Renderer.hpp"


Renderer::Renderer(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel, QObject *parent) :
    mWorker(new RenderWorker(document, instrumentModel, waveModel)),
    mThread(),
    mRendering(false),
    QObject(parent)
{
    mWorker->moveToThread(&mThread);
    connect(&mThread, &QThread::finished, mWorker, &QObject::deleteLater);
    connect(this, &Renderer::rendering, mWorker, &RenderWorker::render);
    mThread.start();
}

Renderer::~Renderer() {
    if (mWorker->isRendering()) {
        mWorker->quitRender();
    }
    mThread.quit();
    mThread.wait();
}


void Renderer::previewWaveform(trackerboy::Note note) {
    mWorker->previewWaveform(note);
    if (!mWorker->isRendering()) {
        emit rendering();
    }
}

void Renderer::previewInstrument(trackerboy::Note note) {
    (void)note;
    // TODO
}

void Renderer::stopPreview() {
    mWorker->stopPreview();
}

