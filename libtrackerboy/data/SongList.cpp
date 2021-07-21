
#include "trackerboy/data/SongList.hpp"

#include <algorithm>
#include <stdexcept>

namespace trackerboy {

SongList::SongList() :
    mContainer()
{
    // always have at least 1 song
    append();
}

void SongList::append() {
    mContainer.emplace_back(std::make_shared<Song>());
}

std::shared_ptr<Song> SongList::appendAndGet() {
    append();
    return mContainer.back();
}

void SongList::duplicate(int index) {
    checkIndex(index);
    // invoke Song copy constructor
    mContainer.emplace_back(std::make_shared<Song>(*mContainer[index]));
}

std::shared_ptr<Song> SongList::get(int index) {
    checkIndex(index);

    return mContainer[index];
}

void SongList::remove(int index) {
    checkIndex(index);

    if (mContainer.size() == 1) {
        throw std::invalid_argument("cannot remove: must have at least 1 song");
    }

    mContainer.erase(mContainer.begin() + index);
}

void SongList::clear() {
    mContainer.clear();
    append();
}

void SongList::moveUp(int index) {
    if (index <= 0 || index >= (int)mContainer.size()) {
        throw std::invalid_argument("index out of bounds");
    }

    auto iter = mContainer.begin() + index;
    std::iter_swap(iter, --iter);

}

void SongList::moveDown(int index) {
    if (index < 0 || index >= (int)mContainer.size() - 1) {
        throw std::invalid_argument("index out of bounds");
    }

    auto iter = mContainer.begin() + index;
    std::iter_swap(iter, ++iter);
}

int SongList::size() {
    return (int)mContainer.size();
}

void SongList::checkIndex(int index) {
    if (index < 0 || index >= (int)mContainer.size()) {
        throw std::invalid_argument("index out of bounds");
    }
}

    
}
