
#pragma once

#include <QWidget>
#include "designer/ui_waveeditor.h"

class WaveEditor : public QWidget, private Ui::WaveEditor {

    Q_OBJECT

public:
    explicit WaveEditor();


};
