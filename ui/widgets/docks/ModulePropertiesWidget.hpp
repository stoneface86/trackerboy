
#pragma once

#include <QBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QWidget>


class ModulePropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit ModulePropertiesWidget(QWidget *parent = nullptr);
    ~ModulePropertiesWidget();


private:

    QBoxLayout mLayout;
        QLineEdit mTitleEdit;
        QLineEdit mArtistEdit;
        QLineEdit mCopyrightEdit;
        QPlainTextEdit mCommentsEdit;

};
