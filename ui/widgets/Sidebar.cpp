
#include "widgets/Sidebar.hpp"

#include <QGroupBox>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent) :
    QWidget(parent),
    mScope(new AudioScope),
    mOrderEditor(new OrderEditor),
    mSongEditor(new SongEditor)
{

    auto layout = new QVBoxLayout;
    layout->addWidget(mScope);

    auto groupbox = new QGroupBox(tr("Song settings"));
    auto groupLayout = new QVBoxLayout;
    groupLayout->addWidget(mSongEditor);
    groupLayout->setMargin(0);
    groupbox->setLayout(groupLayout);
    layout->addWidget(groupbox);

    groupbox = new QGroupBox(tr("Song order"));
    groupLayout = new QVBoxLayout;
    groupLayout->addWidget(mOrderEditor);
    groupLayout->setMargin(0);
    groupbox->setLayout(groupLayout);
    layout->addWidget(groupbox, 1);

    setLayout(layout);
}

AudioScope* Sidebar::scope() {
    return mScope;
}

OrderEditor* Sidebar::orderEditor() {
    return mOrderEditor;
}

SongEditor* Sidebar::songEditor() {
    return mSongEditor;
}
