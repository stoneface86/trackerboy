
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QWidget>


class ModuleSettingsWidget : public QWidget {

    Q_OBJECT

public:

    explicit ModuleSettingsWidget(ModuleDocument &doc, QWidget *parent = nullptr);

private:

    ModuleDocument &mDocument;


    

};
