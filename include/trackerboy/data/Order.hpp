
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

    void resize(size_t size);

    size_t size() const noexcept;

    std::vector<OrderRow> const& data() noexcept;

    //
    // Inserts a pattern at the end of the order
    //
    void insert(OrderRow const& data);

    //
    // Inserts a pattern at the given index
    //
    void insert(uint8_t before, OrderRow const& data);

    //
    // Remove the given pattern. runtime_error is thrown when trying
    // to remove the last pattern.
    //
    void remove(uint8_t pattern);

    //
    // Moves a pattern to a destination index.
    // ie move(2, 4) a, b, c, d, e  =>  a, c, d, e, b
    //
    void move(uint8_t from, uint8_t to);

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
