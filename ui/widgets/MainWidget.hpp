
#pragma once

#include "widgets/PatternEditor.hpp"
#include "widgets/visualizers/AudioScope.hpp"

#include <QHBoxLayout>
#include <QWidget>
#include <QVBoxLayout>

class MainWidget : public QWidget {

    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);

private:

    QVBoxLayout mLayout;
        QHBoxLayout mVisLayout;
            AudioScope mLeftScope;
            AudioScope mRightScope;
        PatternEditor mPatternEditor;


};
