
#include "core/clipboard/PatternClipboard.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>


PatternClipboard::PatternClipboard(QObject *parent) :
    QObject(parent),
    mClip(),
    mSettingClipboard(false)
{
    auto clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &PatternClipboard::parseClipboard);
    parseClipboard();
}

bool PatternClipboard::hasClip() const {
    return mClip.has_value();
}

PatternClip const& PatternClipboard::clip() const {
    return *mClip;
}

void PatternClipboard::setClip(PatternClip &clip) {
    if (!clip.hasData()) {
        return;
    }

    auto mime = new QMimeData;
    clip.toMime(mime);

    mSettingClipboard = true;
    QGuiApplication::clipboard()->setMimeData(mime);

    mClip = std::move(clip);
}

void PatternClipboard::parseClipboard() {
    if (mSettingClipboard) {
        mSettingClipboard = false;
        return;
    }

    auto clipboard = QGuiApplication::clipboard();
    auto mime = clipboard->mimeData();

    PatternClip clip;
    if (clip.fromMime(mime)) {
        mClip = std::move(clip);
    }

}
