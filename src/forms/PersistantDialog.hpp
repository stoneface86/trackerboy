
#pragma once

#include "verdigris/wobjectdefs.h"

#include <QDialog>

//
// QDialog subclass that remembers its geometry on close
//
class PersistantDialog : public QDialog {

    W_OBJECT(PersistantDialog)

public:
    explicit PersistantDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

protected:

    virtual void showEvent(QShowEvent *evt) override;

private:
    Q_DISABLE_COPY(PersistantDialog)

    bool mShown;

};
