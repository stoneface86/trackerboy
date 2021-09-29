
#pragma once


#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>


class AboutDialog : public QDialog {

    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);


private:

    QVBoxLayout mLayout;
        QLabel mLogo;
        QFrame mLine;
        QHBoxLayout mBodyLayout;
            QLabel mIcon;
            QLabel mBody;
        QDialogButtonBox mButtons;


};
