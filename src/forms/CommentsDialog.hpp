
#pragma once

#include "core/Module.hpp"

#include <QDialog>
#include <QPlainTextEdit>

class CommentsDialog : public QDialog {

    Q_OBJECT

public:

    explicit CommentsDialog(Module &mod, QWidget *parent = nullptr);

private slots:

    void reload();

    void commit();

private:

    Q_DISABLE_COPY(CommentsDialog)

    Module &mModule;

    QPlainTextEdit *mEdit;

};
