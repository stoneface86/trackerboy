/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "trackerboy/data/OrderRow.hpp"

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
