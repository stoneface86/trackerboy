
#pragma once

#include "trackerboy/data/Song.hpp"

#include <memory>
#include <vector>

namespace trackerboy {

//
// Container class for a module's list of songs. The song list is just a vector
// of Songs, dynamically allocated using std::shared_ptr. The list will always
// have at least 1 song.
//
class SongList {

    using Container = std::vector<std::shared_ptr<Song>>;

public:

    explicit SongList();
    ~SongList() = default;

    //
    // Creates a new song and appends it to the end of the list. The newly
    // added song get be accessed by calling get(size() - 1).
    //
    void append();

    //
    // Gets the song at the given index. The lifetime of the returned Song
    // is at least the lifetime of this container. If you require the Song
    // to outlive this container, call getShared instead.
    //
    Song* get(int index) const;

    //
    // Gets shared ownership of the song at the given index. Call this instead
    // of get if you need better control of the song's lifetime.
    //
    std::shared_ptr<Song> getShared(int index) const;

    //
    // Duplicates the song at the given index and appends the copy to the
    // end of the list
    //
    void duplicate(int index);

    //
    // Remove the song at the given index from the list
    //
    void remove(int index);

    //
    // Clears all songs. Afterwards the list has 1 new song in the list.
    //
    void clear();

    //
    // Clear the list and append count number of songs
    //
    void clear(int count);

    //
    // Moves the song at the index up one in the list. index must be
    // >= 1 and < size()
    //
    void moveUp(int index);

    //
    // Moves the song at the index down one in the list. index must be
    // >= 0 and < size() - 1.
    //
    void moveDown(int index);

    //
    // Returns the number of songs in this container.
    //
    int size() const;

private:
    void checkIndex(int index) const;


    Container mContainer;

};

}
