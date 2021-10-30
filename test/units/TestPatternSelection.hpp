
#pragma once

#include <QtTest/QtTest>

class TestPatternSelection : public QObject {

    Q_OBJECT

public:

    Q_INVOKABLE TestPatternSelection();

private slots:


    void normalization_data();
    void normalization();

    void translate_data();
    void translate();

    void iteration_data();
    void iteration();

};
