
#pragma once

#include "trackerboy/data/OrderRow.hpp"

#include <cstdint>
#include <vector>

namespace trackerboy {

class Order {

public:

    static constexpr size_t MAX_SIZE = 256;

    Order();

    OrderRow& operator[](uint8_t pattern);
    OrderRow const& operator[](uint8_t pattern) const;

    void resize(size_t size);

    size_t size() const noexcept;

    std::vector<OrderRow> const& data() const noexcept;
    std::vector<OrderRow>& data() noexcept;

    //
    // Inserts a pattern at the end of the order
    //
    void insert(OrderRow const& data);

    //
    // Inserts a pattern at the given index
    //
    void insert(uint8_t before, OrderRow const& data);

    void insert(uint8_t before, OrderRow const *data, size_t count);

    //
    // Remove the given pattern or the given count of patterns starting at pattern.
    // runtime_error is thrown when trying to remove the last pattern.
    //
    void remove(uint8_t pattern, size_t count = 1);

    //
    // Moves a pattern to a destination index.
    // ie move(2, 4) a, b, c, d, e  =>  a, c, d, e, b
    //
    void move(uint8_t from, uint8_t to);

    //
    // Patterns p1 and p2 are swapped with each other
    //
    void swapPatterns(uint8_t p1, uint8_t p2);

    //
    // Sets order data from the given vector.
    //
    void setData(std::vector<OrderRow> &data);

    //
    // Same as above, but the order takes ownership of the vector.
    //
    void setData(std::vector<OrderRow> &&data);

private:
    void assertCanSetData(std::vector<OrderRow> &row);
    void assertCanInsert();

    std::vector<OrderRow> mTable;


};



}
