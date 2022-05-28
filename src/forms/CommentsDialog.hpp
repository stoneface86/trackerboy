
#pragma once

#include "core/Module.hpp"
#include "forms/PersistantDialog.hpp"
#include "verdigris/wobjectdefs.h"

#include <QPlainTextEdit>

class CommentsDialog : public PersistantDialog {

    W_OBJECT(CommentsDialog)

public:

    explicit CommentsDialog(Module &mod, QWidget *parent = nullptr);

private:
    Q_DISABLE_COPY(CommentsDialog)

    void reload();

    void commit();

    Module &mModule;

    QPlainTextEdit *mEdit;

};
