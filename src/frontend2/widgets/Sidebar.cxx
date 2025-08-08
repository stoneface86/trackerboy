
#include "widgets/Sidebar.hxx"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent) {

    auto layout = new QVBoxLayout;

    auto scope = new QLabel("Audio scope");
    scope->setFixedHeight(160);

    auto songGroup = new QGroupBox(tr("Song"));
    auto songGroupLayout = new QVBoxLayout;
    auto songBox = new QComboBox;
    songGroupLayout->addWidget(songBox);
    songGroup->setLayout(songGroupLayout);

    auto tabs = new QTabWidget;
    auto orderTab = new QLabel("Order Editor");
    auto songTab = new QLabel("Song editor");
    tabs->addTab(orderTab, tr("Song Order"));
    tabs->addTab(songTab, tr("Song Settings"));

    layout->addWidget(scope);
    layout->addWidget(songGroup);
    layout->addWidget(tabs, 1);
    setLayout(layout);
}
