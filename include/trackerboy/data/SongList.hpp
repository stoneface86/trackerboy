
#pragma once

#include "trackerboy/data/Song.hpp"

#include <memory>
#include <vector>

namespace trackerboy {

class SongList {

    using Container = std::vector<std::shared_ptr<Song>>;

public:

    explicit SongList();
    ~SongList() = default;

    void append();

    //
    // Gets the song at the given index. The lifetime of the returned Song
    // is at least the lifetime of this container. If you require the Song
    // to outlive this container, call getShared instead.
    //
    Song* get(int index) const;

    //
    // Gets shared ownership of the song at the given index.
    //
    std::shared_ptr<Song> getShared(int index) const;

    void duplicate(int index);

    void remove(int index);

    void clear();

    void moveUp(int index);

    void moveDown(int index);

    int size();

private:
    void checkIndex(int index) const;


    Container mContainer;

};

}
