
#include "trackerboy/data/Order.hpp"

#include <algorithm>
#include <stdexcept>

namespace trackerboy {

Order::Order() :
    mTable()
{
    mTable.push_back({ 0 });
}

OrderRow& Order::operator[](uint8_t pattern) {
    return mTable[pattern];
}

void Order::resize(size_t size) {
    if (size == 0) {
        throw std::runtime_error("order size must be nonzero");
    }

    mTable.resize(size);
}

size_t Order::size() const noexcept {
    return mTable.size();
}

std::vector<OrderRow> const& Order::data() const noexcept {
    return mTable;
}

std::vector<OrderRow>& Order::data() noexcept {
    return mTable;
}

void Order::insert(OrderRow const& data) {
    assertCanInsert();
    mTable.push_back(data);
}

void Order::insert(uint8_t before, OrderRow const& data) {
    assertCanInsert();
    mTable.insert(mTable.begin() + before, data);
}

void Order::insert(uint8_t before, OrderRow const *data, size_t count) {
    auto newsize = mTable.size() + count;
    if (newsize > MAX_SIZE) {
        throw std::runtime_error("cannot insert: maximum number of patterns");
    }
    mTable.insert(mTable.begin() + before, data, data + count);
}

//
// moves an element in a container to a new position, while reordering
// all elements within range.
//
// example
// let vec = {1, 2, 3, 4};
//
// moveElement(vec, 1, 3);
// vec -> { 2, 3, 4, 1 }
//
// moveElement(vec, 2, 0);
// vec -> { 3, 1, 2, 4 }
//
//
template <class Container>
void moveElement(Container &cont, size_t from, size_t to) {
    if (from == to) {
        return;
    } else if (from < to) {
        auto begin = cont.begin();
        auto start = begin + from;
        std::rotate(start, start + 1, begin + to + 1);
    } else {
        auto end = cont.rend();
        auto start = end - from;
        std::rotate(start, start - 1, end - to - 1);
    }
}

void Order::move(uint8_t from, uint8_t to) {
    moveElement(mTable, from, to);
}

void Order::remove(uint8_t pattern, size_t count) {
    if (count) {
        if (mTable.size() <= count) {
            throw std::runtime_error("cannot remove: Order must have at least 1 pattern");
        }
        auto iter = mTable.begin() + pattern;
        mTable.erase(iter, iter + count);
    }
}

void Order::swapPatterns(uint8_t p1, uint8_t p2) {
    auto begin = mTable.begin();
    std::iter_swap(begin + p1, begin + p2);
}

void Order::setData(std::vector<OrderRow> &data) {
    assertCanSetData(data);
    mTable = data;
}

void Order::setData(std::vector<OrderRow> &&data) {
    assertCanSetData(data);
    mTable = std::move(data);
}

void Order::assertCanInsert() {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot insert: maximum number of patterns");
    }
}

void Order::assertCanSetData(std::vector<OrderRow> &data) {
    auto sz = data.size();
    if (sz == 0) {
        throw std::invalid_argument("cannot set data: order must have at least 1 paterrn");
    } else if (sz > MAX_SIZE) {
        throw std::invalid_argument("cannot set data: too many patterns");
    }
}

}
