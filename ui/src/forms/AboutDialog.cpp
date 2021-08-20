
#include "forms/AboutDialog.hpp"

#include "trackerboy/version.hpp"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mLayout(),
    mLogo(),
    mLine(),
    mBodyLayout(),
    mIcon(),
    mBody(),
    mButtons()
{
    setModal(true);
    setWindowTitle(tr("About"));

    mBodyLayout.addWidget(&mIcon);
    mBodyLayout.addWidget(&mBody, 1);

    mLayout.addWidget(&mLogo);
    mLayout.addWidget(&mLine);
    mLayout.addLayout(&mBodyLayout, 1);
    mLayout.addWidget(&mButtons);
    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(&mLayout);

    mLogo.setPixmap(QPixmap(QStringLiteral(":/images/logo.png")));
    mIcon.setPixmap(QPixmap(QStringLiteral(":/icons/app/appicon-48.png")));

    mLine.setFrameShape(QFrame::HLine);
    mLine.setFrameShadow(QFrame::Sunken);

    mIcon.setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    mBody.setTextFormat(Qt::RichText);
    mBody.setTextInteractionFlags(Qt::TextBrowserInteraction);
    mBody.setOpenExternalLinks(true);
    mBody.setText(QStringLiteral(
R"bodystr(
<html><head/>
<body>
<h1>Trackerboy v%1.%2.%3</h1>
<h3> A Gameboy/Gameboy Color music tracker</h3>
<h3>Copyright (C) 2019-2021 stoneface86</h3>
<br/>
<br/>
<span>Repo - <a href="https://github.com/stoneface86/trackerboy">https://github.com/stoneface86/trackerboy</a></span>
<br/>
<p>This software is licensed under the MIT License.</p>
</body>
</html>
)bodystr").arg(trackerboy::VERSION.major)
          .arg(trackerboy::VERSION.minor)
          .arg(trackerboy::VERSION.patch));

    mButtons.addButton(QDialogButtonBox::Close);

    connect(&mButtons, &QDialogButtonBox::rejected, this, &AboutDialog::reject);
}
