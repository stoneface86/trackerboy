
#pragma once

#include <QMainWindow>

#pragma warning(push, 0)
#include "designer/ui_mainwindow.h"
#pragma warning(pop)

class MainWindow : public QMainWindow, private Ui::MainWindow {

    Q_OBJECT

public:
    explicit MainWindow();


};
