
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QWidget>


class InstrumentsWidget : public QWidget {

    Q_OBJECT

public:

    explicit InstrumentsWidget(ModuleDocument &doc, QWidget *parent = nullptr);

private:

    ModuleDocument &mDocument;




};
