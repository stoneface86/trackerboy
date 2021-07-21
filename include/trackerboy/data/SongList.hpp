
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

    std::shared_ptr<Song> appendAndGet();

    std::shared_ptr<Song> get(int index);

    void duplicate(int index);

    void remove(int index);

    void clear();

    void moveUp(int index);

    void moveDown(int index);

    int size();

private:
    void checkIndex(int index);


    Container mContainer;

};

}
