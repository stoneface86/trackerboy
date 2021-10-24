// Note: this test class is an example/template and is not compiled


#pragma once

#include <QtTest/QtTest>

class TestFoo : public QObject {

    Q_OBJECT

public:

    // constructor must be Q_INVOKABLE so we can instantiate by test name
    Q_INVOKABLE TestFoo();

private slots:

    // put your test cases here

};
