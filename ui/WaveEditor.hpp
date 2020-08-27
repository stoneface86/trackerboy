
#pragma once

#include <QWidget>
#include "ui_waveeditor.h"

class WaveEditor : public QWidget, private Ui::WaveEditor {

    Q_OBJECT

public:
    explicit WaveEditor();


};
