
#include "utils/utils.hpp"

#include <Qt>
#include <QThread>
#include <QSplitter>

std::optional<uint8_t> keyToHex(int const key) {
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return (uint8_t)(key - Qt::Key_0);
    } else if (key >= Qt::Key_A && key <= Qt::Key_F) {
        return (uint8_t)(key - Qt::Key_A + 0xA);
    } else {
        return std::nullopt;
    }
}

uint8_t replaceNibble(uint8_t value, uint8_t nibble, bool highNibble) {
    if (highNibble) {
        return (value & 0x0F) | (nibble << 4);
    } else {
        return (value & 0xF0) | (nibble);
    }
}

bool objectInCurrentThread(QObject const& obj) {
    return obj.thread() == QThread::currentThread();
}

void rationSplitter(QSplitter &splitter, float ratio) {
    auto total = splitter.orientation() == Qt::Horizontal ? splitter.width() : splitter.height();
    auto first = (int)(total * ratio);
    auto second = total - first;
    splitter.setSizes({ first, second });
}
