
#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>

#include "trackerboy.hpp"
#include "SynthWorker.hpp"


namespace instedit {




class InstEditor : public QWidget {

    Q_OBJECT

public:

    InstEditor(QWidget *parent = nullptr);
    ~InstEditor();

private slots:
    void channelSelected(int index);
    void play();
    void workerFinished();
    
private:

    QPlainTextEdit *programEdit;
    QPlainTextEdit *outputEdit;
    QPushButton *playButton;
    QCheckBox *loopCheckbox;
    QLineEdit *frequencyEdit;
    SynthWorker *worker;
    
    std::vector<trackerboy::Instruction>* programTable[4];
    trackerboy::InstrumentRuntime* runtimeTable[4];
    trackerboy::TrackId currentTrackId;

    QStringList programList;
    QStringList outputList;

    bool playing;

    bool parse();

};


}