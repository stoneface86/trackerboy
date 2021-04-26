
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QWidget>


class WaveformsWidget : public QWidget {

    Q_OBJECT

public:

    explicit WaveformsWidget(ModuleDocument &doc, QWidget *parent = nullptr);

private:

    ModuleDocument &mDocument;




};
