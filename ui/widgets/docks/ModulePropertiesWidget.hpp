
#pragma once

#include <QBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QWidget>

//
// Module properties dock widget. Allows the user to set the title, artist, copyright
// and comments for the module.
//
class ModulePropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit ModulePropertiesWidget(QWidget *parent = nullptr);
    ~ModulePropertiesWidget();


private:
    Q_DISABLE_COPY(ModulePropertiesWidget)

    QBoxLayout mLayout;
        QLineEdit mTitleEdit;
        QLineEdit mArtistEdit;
        QLineEdit mCopyrightEdit;
        QPlainTextEdit mCommentsEdit;

};
