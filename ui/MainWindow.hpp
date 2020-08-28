
#pragma once

#include <QMainWindow>
#include "designer/ui_mainwindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow {

    Q_OBJECT

public:
    explicit MainWindow();


};
