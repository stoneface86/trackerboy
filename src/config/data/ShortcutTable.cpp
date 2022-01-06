
#include "config/data/ShortcutTable.hpp"
#include "config/data/keys.hpp"

#include <QKeySequence>
#include <QSettings>

#define TU ShortcutTableTU
namespace TU {

static std::array const SHORTCUT_KEYS = {
    QStringLiteral("PrevInstrument"),
    QStringLiteral("NextInstrument"),
    QStringLiteral("PrevPattern"),
    QStringLiteral("NextPattern"),
    QStringLiteral("IncOctave"),
    QStringLiteral("DecOctave"),
    QStringLiteral("Play")
};

static_assert(SHORTCUT_KEYS.size() == ShortcutTable::Count, "table size mismatch");

}

ShortcutTable::ShortcutTable() noexcept :
    mTable()
{
}

std::array<ShortcutTable::KeyPack, ShortcutTable::Count> const ShortcutTable::DEFAULTS{ {
    { Qt::CTRL | Qt::Key_Left, 0, 0, 0 },
    { Qt::CTRL | Qt::Key_Right, 0, 0, 0 },
    { Qt::CTRL | Qt::Key_Up, 0, 0, 0 },
    { Qt::CTRL | Qt::Key_Down, 0, 0, 0 },
    { Qt::CTRL | Qt::Key_BracketRight, 0, 0, 0 },
    { Qt::CTRL | Qt::Key_BracketLeft, 0, 0, 0 },
    { Qt::Key_Return, 0, 0, 0 }
} };

void ShortcutTable::set(Shortcut shortcut, QKeySequence const& seq) noexcept {
    mTable[shortcut] = toPack(seq);
}

QKeySequence ShortcutTable::get(Shortcut shortcut) const noexcept {
    return toSequence(mTable[shortcut]);
}

QKeySequence ShortcutTable::getDefault(Shortcut shortcut) noexcept {
    return toSequence(DEFAULTS[shortcut]);
}

void ShortcutTable::readSettings(QSettings &settings) noexcept {
    settings.beginGroup(Keys::Shortcuts);

    for (int i = 0; i < Count; ++i) {
        auto seqstr = settings.value(TU::SHORTCUT_KEYS[i]).toString();
        if (seqstr.isEmpty()) {
            mTable[i] = DEFAULTS[i];
        } else {
            mTable[i] = toPack(QKeySequence(seqstr, QKeySequence::PortableText));
        }
    }

    settings.endGroup();
}

void ShortcutTable::writeSettings(QSettings &settings) noexcept {
    settings.beginGroup(Keys::Shortcuts);

    for (int i = 0; i < Count; ++i) {
        settings.setValue(TU::SHORTCUT_KEYS[i], toSequence(mTable[i]).toString(QKeySequence::PortableText));
    }

    settings.endGroup();
}

QKeySequence ShortcutTable::toSequence(KeyPack const& pack) noexcept {
    return { pack[0], pack[1], pack[2], pack[3] };
}

ShortcutTable::KeyPack ShortcutTable::toPack(QKeySequence const& seq) noexcept {
    return { seq[0], seq[1], seq[2], seq[3] };
}

#undef TU
