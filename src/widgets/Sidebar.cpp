
#include "widgets/Sidebar.hpp"
#include "utils/connectutils.hpp"
#include "utils/IconLocator.hpp"

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


    mNextAction = new QAction(tr("Next song"), this);
    mNextAction->setStatusTip(tr("Selects the next song in the list"));
    mNextAction->setIcon(IconLocator::get(Icons::next));
    connectActionToThis(mNextAction, nextSong);

    mPrevAction = new QAction(tr("Previous song"), this);
    mPrevAction->setStatusTip(tr("Selects the previous song in the list"));
    mPrevAction->setIcon(IconLocator::get(Icons::prev));
    connectActionToThis(mPrevAction, previousSong);

    lazyconnect(&mod, reloaded, this, reload);
    connect(mSongChooser, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this, &mod](int index) {
            if (index != -1) {
                // index should never be -1 with our model but check just in case
                mod.setSong(index);
            }
            updateActions();
        });

    lazyconnect(&songListModel, rowsInserted, this, updateActions);
    lazyconnect(&songListModel, rowsRemoved, this, updateActions);
    lazyconnect(&songListModel, rowsMoved, this, updateActions);


    updateActions();
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

QAction* Sidebar::previousSongAction() {
    return mPrevAction;
}

QAction* Sidebar::nextSongAction() {
    return mNextAction;
}

void Sidebar::nextSong() {
    mSongChooser->setCurrentIndex(mSongChooser->currentIndex() + 1);
}

void Sidebar::previousSong() {
    mSongChooser->setCurrentIndex(mSongChooser->currentIndex() - 1);
}

void Sidebar::reload() {
    {
        QSignalBlocker blocker(mSongChooser);
        mSongChooser->setCurrentIndex(0);
    }
    updateActions();
}

void Sidebar::updateActions() {
    auto const current = mSongChooser->currentIndex();

    mPrevAction->setEnabled(current > 0);
    mNextAction->setEnabled(current != -1 && current != mSongChooser->count() - 1);
}
