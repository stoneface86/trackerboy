#pragma once

#include "Config.hpp"

#include <QDialog>
#include <QSettings>
#include <QShowEvent>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog {

    Q_OBJECT

public:

    //
    // Configuration categories flags. When applying settings, only the categories
    // with changes will get applied.
    //
    enum Category {
        CategoryNone = 0,
        CategorySound = 1,

        CategoryAll = CategorySound
    };
    Q_DECLARE_FLAGS(Categories, Category);

    ConfigDialog(Config &config, QWidget *parent = nullptr);
    ~ConfigDialog();

    void accept() override;

    void reject() override;

    void resetControls();

signals:
    // emitted when changes in the Config must be applied
    void applied(Categories categories);

protected:

    void showEvent(QShowEvent *evt) override;

private slots:
    void apply();

    void bufferSizeSliderChanged(int value);
    void volumeSliderChanged(int value);
    void samplerateActivated(int index);


    
private:
    void setDirty(Category flag);

    void clean();

    Ui::ConfigDialog *mUi;

    Config &mConfig;
    // configuration categories that are "dirty". A dirty category
    // contains settings changed by the user and must be applied when the
    // user clicks the OK or Apply button.
    Categories mDirty;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(ConfigDialog::Categories);
