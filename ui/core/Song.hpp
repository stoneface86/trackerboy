
#pragma once

#include "core/Module.hpp"

#include <QObject>

class Song : public QObject {

    Q_OBJECT

public:
    explicit Song(Module &mod, QObject *parent = nullptr);
    virtual ~Song() = default;


private:

    Module &mModule;
    QUndoStack mUndoStack;

};

