
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QWidget>


class PatternsWidget : public QWidget {

    Q_OBJECT

public:

    explicit PatternsWidget(ModuleDocument &doc, QWidget *parent = nullptr);

private:

    ModuleDocument &mDocument;




};
