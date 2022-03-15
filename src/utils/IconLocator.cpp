
#include "utils/IconLocator.hpp"

#include <type_traits>

namespace IconLocator {

QIcon get(Icons type) {
    QIcon icon;
    switch (type) {
        case Icons::fileNew:
            icon.addFile(QStringLiteral(":/icons/fileNew.png"));
            break;
        case Icons::fileOpen:
            icon.addFile(QStringLiteral(":/icons/fileOpen.png"));
            break;
        case Icons::fileSave:
            icon.addFile(QStringLiteral(":/icons/save.png"));
            icon.addFile(QStringLiteral(":/icons/save-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::fileConfig:
            icon.addFile(QStringLiteral(":/icons/fileConfig.png"));
            break;
        case Icons::editUndo:
            icon.addFile(QStringLiteral(":/icons/editUndo.png"));
            icon.addFile(QStringLiteral(":/icons/editUndo-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::editRedo:
            icon.addFile(QStringLiteral(":/icons/editRedo.png"));
            icon.addFile(QStringLiteral(":/icons/editRedo-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::editCut:
            icon.addFile(QStringLiteral(":/icons/editCut.png"));
            icon.addFile(QStringLiteral(":/icons/editCut-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::editCopy:
            icon.addFile(QStringLiteral(":/icons/editCopy.png"));
            icon.addFile(QStringLiteral(":/icons/editCopy-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::editPaste:
            icon.addFile(QStringLiteral(":/icons/editPaste.png"));
            icon.addFile(QStringLiteral(":/icons/editPaste-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::itemAdd:
            icon.addFile(QStringLiteral(":/icons/itemAdd.png"));
            icon.addFile(QStringLiteral(":/icons/itemAdd-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::itemRemove:
            icon.addFile(QStringLiteral(":/icons/itemRemove.png"));
            icon.addFile(QStringLiteral(":/icons/itemRemove-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::itemDuplicate:
            icon.addFile(QStringLiteral(":/icons/itemDuplicate.png"));
            icon.addFile(QStringLiteral(":/icons/itemDuplicate-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::itemImport:
            icon.addFile(QStringLiteral(":/icons/itemImport.png"));
            icon.addFile(QStringLiteral(":/icons/itemImport-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::itemEdit:
            icon.addFile(QStringLiteral(":/icons/itemEdit.png"));
            icon.addFile(QStringLiteral(":/icons/itemEdit-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerPlay:
            icon.addFile(QStringLiteral(":/icons/trackerPlay.png"));
            icon.addFile(QStringLiteral(":/icons/trackerPlay-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerRestart:
            icon.addFile(QStringLiteral(":/icons/trackerRestart.png"));
            icon.addFile(QStringLiteral(":/icons/trackerRestart-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerPlayRow:
            icon.addFile(QStringLiteral(":/icons/trackerPlayRow.png"));
            icon.addFile(QStringLiteral(":/icons/trackerPlayRow-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerStepRow:
            icon.addFile(QStringLiteral(":/icons/trackerStepRow.png"));
            icon.addFile(QStringLiteral(":/icons/trackerStepRow-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerStop:
            icon.addFile(QStringLiteral(":/icons/trackerStop.png"));
            icon.addFile(QStringLiteral(":/icons/trackerStop-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerRecord:
            icon.addFile(QStringLiteral(":/icons/trackerRecord.png"));
            icon.addFile(QStringLiteral(":/icons/trackerRecord-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::trackerRepeat:
            icon.addFile(QStringLiteral(":/icons/trackerRepeat.png"));
            icon.addFile(QStringLiteral(":/icons/trackerRepeat-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::ch1:
            icon.addFile(QStringLiteral(":/icons/ch1.png"));
            break;
        case Icons::ch2:
            icon.addFile(QStringLiteral(":/icons/ch2.png"));
            break;
        case Icons::ch3:
            icon.addFile(QStringLiteral(":/icons/ch3.png"));
            break;
        case Icons::ch4:
            icon.addFile(QStringLiteral(":/icons/ch4.png"));
            break;
        case Icons::increment:
            icon.addFile(QStringLiteral(":/icons/increment.png"));
            break;
        case Icons::decrement:
            icon.addFile(QStringLiteral(":/icons/decrement.png"));
            break;
        case Icons::changeAll:
            icon.addFile(QStringLiteral(":/icons/orderChangeAll.png"));
            break;
        case Icons::moveUp:
            icon.addFile(QStringLiteral(":/icons/moveUp.png"));
            icon.addFile(QStringLiteral(":/icons/moveUp-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::moveDown:
            icon.addFile(QStringLiteral(":/icons/moveDown.png"));
            icon.addFile(QStringLiteral(":/icons/moveDown-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::next:
            icon.addFile(QStringLiteral(":/icons/next.png"));
            icon.addFile(QStringLiteral(":/icons/next-disabled.png"), QSize(), QIcon::Disabled);
            break;
        case Icons::prev:
            icon.addFile(QStringLiteral(":/icons/prev.png"));
            icon.addFile(QStringLiteral(":/icons/prev-disabled.png"), QSize(), QIcon::Disabled);
            break;
        default:
            icon.addFile(QStringLiteral(":/icons/placeholder.png"));
            icon.addFile(QStringLiteral(":/icons/placeholder-disabled.png"), QSize(), QIcon::Disabled);
            break;
    }

    return icon;
}

QIcon getAppIcon() {
    QIcon appIcon;
    appIcon.addFile(":/icons/app/appicon-16.png");
    appIcon.addFile(":/icons/app/appicon-24.png");
    appIcon.addFile(":/icons/app/appicon-32.png");
    appIcon.addFile(":/icons/app/appicon-48.png");
    appIcon.addFile(":/icons/app/appicon-256.png");
    return appIcon;
}

}
