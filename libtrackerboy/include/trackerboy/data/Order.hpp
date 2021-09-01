
#pragma once

#include "trackerboy/data/OrderRow.hpp"

#include <cstdint>
#include <vector>

namespace trackerboy {

class Order {

public:

    static constexpr int MAX_SIZE = 256;

    Order();

    OrderRow& operator[](int pattern);
    OrderRow const& operator[](int pattern) const;

    void resize(int size);

    int size() const noexcept;

    std::vector<OrderRow> const& data() const noexcept;
    std::vector<OrderRow>& data() noexcept;

    //
    // Calculates an OrderRow with each track id being the smallest unused in
    // the order
    //
    OrderRow nextUnused() const noexcept;

    //
    // Inserts a pattern at the end of the order
    //
    void insert(OrderRow const& data);

    //
    // Inserts a pattern at the given index
    //
    void insert(int before, OrderRow const& data);

    void insert(int before, OrderRow const *data, int count);

    //
    // Remove the given pattern or the given count of patterns starting at pattern.
    // runtime_error is thrown when trying to remove the last pattern.
    //
    void remove(int pattern, int count = 1);

    //
    // Moves a pattern to a destination index.
    // ie move(2, 4) a, b, c, d, e  =>  a, c, d, e, b
    //
    void move(int from, int to);

    //
    // Patterns p1 and p2 are swapped with each other
    //
    void swapPatterns(int p1, int p2);

    //
    // Sets order data from the given vector.
    //
    void setData(std::vector<OrderRow> const& data);

    //
    // Same as above, but the order takes ownership of the vector.
    //
    void setData(std::vector<OrderRow> &&data);

private:
    void assertCanSetData(std::vector<OrderRow> const& row);
    void assertCanInsert();

    void badIndex();

    void checkIndex(int index);

    std::vector<OrderRow> mTable;


};



}
