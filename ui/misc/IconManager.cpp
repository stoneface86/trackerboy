
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

QIcon& IconManager::getIcon(Icons icon) {

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
                iconRef.addFile(QStringLiteral(":/icons/fileSave.png"));
                break;
            case Icons::fileConfig:
                iconRef = instance.mPlaceholder;
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
                iconRef = instance.mPlaceholder;
                break;
            case Icons::editCopy:
                iconRef = instance.mPlaceholder;
                break;
            case Icons::editPaste:
                iconRef = instance.mPlaceholder;
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
            case Icons::itemExport:
                iconRef.addFile(QStringLiteral(":/icons/itemExport.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemExport-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::itemEdit:
                iconRef.addFile(QStringLiteral(":/icons/itemEdit.png"));
                iconRef.addFile(QStringLiteral(":/icons/itemEdit-disabled.png"), QSize(), QIcon::Disabled);
                break;
            case Icons::trackerPlay:
                iconRef.addFile(QStringLiteral(":/icons/trackerPlay.png"));
                break;
            case Icons::trackerPlayPattern:
                iconRef.addFile(QStringLiteral(":/icons/trackerPlayPattern.png"));
                break;
            case Icons::trackerStop:
                iconRef.addFile(QStringLiteral(":/icons/trackerStop.png"));
                break;
            case Icons::trackerEdit:
                iconRef.addFile(QStringLiteral(":/icons/trackerEdit.png"));
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
            case Icons::moveUp:
                iconRef = instance.mPlaceholder;
                break;
            case Icons::moveDown:
                iconRef = instance.mPlaceholder;
                break;
            case Icons::previous:
                iconRef = instance.mPlaceholder;
                break;
            case Icons::next:
                iconRef = instance.mPlaceholder;
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


