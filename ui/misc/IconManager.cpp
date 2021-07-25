
#include "misc/IconManager.hpp"

#include <type_traits>

using IconsIntegral = std::underlying_type<Icons>::type;

IconManager::IconManager() {
    mPlaceholder.addFile(QStringLiteral(":/icons/placeholder.png"));
    mPlaceholder.addFile(QStringLiteral(":/icons/placeholder-disabled.png"), QSize(), QIcon::Disabled);

}

QIcon IconManager::getAppIcon() {
    QIcon appIcon;
    appIcon.addFile(":/icons/app/appicon-16.png");
    appIcon.addFile(":/icons/app/appicon-24.png");
    appIcon.addFile(":/icons/app/appicon-32.png");
    appIcon.addFile(":/icons/app/appicon-48.png");
    appIcon.addFile(":/icons/app/appicon-256.png");
    return appIcon;
}

QIcon const& IconManager::getIcon(Icons icon) {

    auto &instance = getInstance();
    
    if (icon == Icons::noIcon) {
        return instance.mPlaceholder;
    }

    auto &iconRef = instance.mIconTable[static_cast<IconsIntegral>(icon)];

    if (iconRef.isNull()) {
        // load it
        switch (icon) {
            case Icons::fileNew:
                iconRef.addFile(QStringLiteral(":/icons/fileNew.png"));
                break;
            case Icons::fileOpen:
                iconRef.addFile(QStringLiteral(":/icons/fileOpen.png"));
                break;
            case Icons::fileSave:
                iconRef.addFile(QStringLiteral(":/icons/save.png"));
                iconRef.addFile(QStringLiteral(":/icons/save-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::fileConfig:
                iconRef.addFile(QStringLiteral(":/icons/fileConfig.png"));
                break;
            case Icons::editUndo:
                iconRef.addFile(QStringLiteral(":/icons/editUndo.png"));
                iconRef.addFile(QStringLiteral(":/icons/editUndo-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::editRedo:
                iconRef.addFile(QStringLiteral(":/icons/editRedo.png"));
                iconRef.addFile(QStringLiteral(":/icons/editRedo-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::editCut:
                iconRef.addFile(QStringLiteral(":/icons/editCut.png"));
                iconRef.addFile(QStringLiteral(":/icons/editCut-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::editCopy:
                iconRef.addFile(QStringLiteral(":/icons/editCopy.png"));
                iconRef.addFile(QStringLiteral(":/icons/editCopy-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::editPaste:
                iconRef.addFile(QStringLiteral(":/icons/editPaste.png"));
                iconRef.addFile(QStringLiteral(":/icons/editPaste-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemAdd:
                iconRef.addFile(QStringLiteral(":/icons/itemAdd.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemAdd-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemRemove:
                iconRef.addFile(QStringLiteral(":/icons/itemRemove.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemRemove-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemDuplicate:
                iconRef.addFile(QStringLiteral(":/icons/itemDuplicate.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemDuplicate-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemImport:
                iconRef.addFile(QStringLiteral(":/icons/itemImport.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemImport-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemEdit:
                iconRef.addFile(QStringLiteral(":/icons/itemEdit.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemEdit-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerPlay:
                iconRef.addFile(QStringLiteral(":/icons/trackerPlay.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerPlay-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerRestart:
                iconRef.addFile(QStringLiteral(":/icons/trackerRestart.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerRestart-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerPlayRow:
                iconRef.addFile(QStringLiteral(":/icons/trackerPlayRow.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerPlayRow-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerStepRow:
                iconRef.addFile(QStringLiteral(":/icons/trackerStepRow.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerStepRow-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerStop:
                iconRef.addFile(QStringLiteral(":/icons/trackerStop.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerStop-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerRecord:
                iconRef.addFile(QStringLiteral(":/icons/trackerRecord.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerRecord-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerRepeat:
                iconRef.addFile(QStringLiteral(":/icons/trackerRepeat.png"));
                iconRef.addFile(QStringLiteral(":/icons/trackerRepeat-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::ch1:
                iconRef.addFile(QStringLiteral(":/icons/ch1.png"));
                break;
            case Icons::ch2:
                iconRef.addFile(QStringLiteral(":/icons/ch2.png"));
                break;
            case Icons::ch3:
                iconRef.addFile(QStringLiteral(":/icons/ch3.png"));
                break;
            case Icons::ch4:
                iconRef.addFile(QStringLiteral(":/icons/ch4.png"));
                break;
            case Icons::increment:
                iconRef.addFile(QStringLiteral(":/icons/increment.png"));
                break;
            case Icons::decrement:
                iconRef.addFile(QStringLiteral(":/icons/decrement.png"));
                break;
            case Icons::moveUp:
                iconRef.addFile(QStringLiteral(":/icons/moveUp.png"));
                iconRef.addFile(QStringLiteral(":/icons/moveUp-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::moveDown:
                iconRef.addFile(QStringLiteral(":/icons/moveDown.png"));
                iconRef.addFile(QStringLiteral(":/icons/moveDown-disabled.png"), QSize(), QIcon::Disabled);
                break;
            default:
                break;
        }

    }

    return iconRef;
}


IconManager& IconManager::getInstance() noexcept {
    static IconManager manager;
    return manager;
}


