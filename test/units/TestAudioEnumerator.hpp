
#pragma once



#include <QtTest/QtTest>

class TestAudioEnumerator : public QObject {

    Q_OBJECT

public:

    Q_INVOKABLE TestAudioEnumerator();

private slots:

    void deviceSerialization();

    void populate();

};
