
#include "widgets/Sidebar.hpp"
#include "core/misc/connectutils.hpp"

#include <QGroupBox>
#include <QVBoxLayout>

Sidebar::Sidebar(
    Module &mod,
    PatternModel &patternModel,
    SongListModel &songListModel,
    SongModel &songModel,
    QWidget *parent
) :
    QWidget(parent),
    mScope(new AudioScope),
    mOrderEditor(new OrderEditor(patternModel)),
    mSongEditor(new SongEditor(songModel)),
    mSongChooser(new QComboBox)
{

    auto layout = new QVBoxLayout;
    layout->addWidget(mScope);

    auto groupbox = new QGroupBox(tr("Song"));
    auto groupLayout = new QVBoxLayout;
    mSongChooser->setModel(&songListModel);
    groupLayout->addWidget(mSongChooser, 1);
    groupbox->setLayout(groupLayout);
    layout->addWidget(groupbox);

    groupbox = new QGroupBox(tr("Song settings"));
    groupLayout = new QVBoxLayout;
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

    lazyconnect(&mod, reloaded, this, reload);
    connect(mSongChooser, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [&mod](int index) {
            if (index != -1) {
                // index should never be -1 with our model but check just in case
                mod.setSong(index);
            }
        });
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

void Sidebar::reload() {
    QSignalBlocker blocker(mSongChooser);
    mSongChooser->setCurrentIndex(0);
}
