
#pragma once

#include <QCoreApplication>
#include <QKeySequence>

class QSettings;

#include <array>

//
// Configuration data for user configured shortcuts.
//
class ShortcutTable {

public:

    enum Shortcut {
        PrevInstrument,
        NextInstrument,
        PrevPattern,
        NextPattern,
        IncOctave,
        DecOctave,
        Play,

        Count
    };

    explicit ShortcutTable() noexcept;

    void set(Shortcut shortcut, QKeySequence const& seq) noexcept;

    QKeySequence get(Shortcut shortcut) const noexcept;

    static QKeySequence getDefault(Shortcut shortcut) noexcept;

    void readSettings(QSettings &settings) noexcept;

    void writeSettings(QSettings &settings) noexcept;


private:

    // use this instead of QKeySequence to avoid heap allocation
    using KeyPack = std::array<int, 4>;

    static inline QKeySequence toSequence(KeyPack const& pack) noexcept;
    static inline KeyPack toPack(QKeySequence const& seq) noexcept;

    static std::array<KeyPack, Count> const DEFAULTS;


    std::array<KeyPack, Count> mTable;


};
