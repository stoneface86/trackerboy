
#include "misc/SongActions.hpp"
#include "misc/IconManager.hpp"

SongActions::SongActions() {
    add.setIcon(IconManager::getIcon(Icons::itemAdd));
    remove.setIcon(IconManager::getIcon(Icons::itemRemove));
    duplicate.setIcon(IconManager::getIcon(Icons::itemDuplicate));
    moveUp.setIcon(IconManager::getIcon(Icons::moveUp));
    moveDown.setIcon(IconManager::getIcon(Icons::moveDown));
}

