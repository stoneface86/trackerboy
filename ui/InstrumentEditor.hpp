
#pragma once

#include <QDialog>

#pragma warning(push, 0)
#include "designer/ui_InstrumentEditor.h"
#pragma warning(pop)

class InstrumentEditor : public QDialog, private Ui::InstrumentEditor {

    Q_OBJECT

public:
    InstrumentEditor(QWidget *parent = nullptr);

private slots:
    void onChannelSelect(int channel);
    void envelopeChanged(int value);
    void envelopeEditChanged(int value);

private:
    //void updateEnvelopeEditor();

    bool mUpdating;

};
