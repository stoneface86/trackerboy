
#include "widgets/Sidebar.hxx"
#include "Sidebar/SongSettingsWidget.hxx"
#include "utils/connectutils.hxx"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

Sidebar::Sidebar(SongListModel *listModel, SongModel *songModel,
                 QWidget *parent)
    : QWidget(parent) {

    auto layout = new QVBoxLayout;

    auto scope = new QLabel("Audio scope");
    scope->setFixedHeight(160);

    auto songGroup = new QGroupBox(tr("Song"));
    auto songGroupLayout = new QVBoxLayout;
    _songCombo = new QComboBox;
    _songCombo->setModel(listModel);
    songGroupLayout->addWidget(_songCombo);
    songGroup->setLayout(songGroupLayout);

    auto tabs = new QTabWidget;
    auto orderTab = new QLabel("Order Editor");
    auto songTab = new SongSettingsWidget(songModel);
    tabs->addTab(orderTab, tr("Song Order"));
    tabs->addTab(songTab, tr("Song Settings"));

    layout->addWidget(scope);
    layout->addWidget(songGroup);
    layout->addWidget(tabs, 1);
    setLayout(layout);

    _document = songModel->document();
    lazyconnect(_songCombo, activated, _document, selectSong);
    lazyconnect(_document, songChanged, _songCombo, setCurrentIndex);
    lazyconnect(listModel, modelReset, this, songListReset);
}

void Sidebar::songListReset() {
    _songCombo->setCurrentIndex(_document->song());
}
