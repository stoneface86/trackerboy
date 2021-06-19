
#include "trackerboy/data/Order.hpp"

#include <algorithm>
#include <stdexcept>

namespace trackerboy {

Order::Order() :
    mTable()
{
    mTable.push_back({ 0 });
}

OrderRow& Order::operator[](int pattern) {
    return mTable[pattern];
}

OrderRow const& Order::operator[](int pattern) const {
    return mTable[pattern];
}

void Order::resize(int size) {
    if (size <= 0) {
        throw std::runtime_error("order size must be nonzero");
    }

    mTable.resize(size);
}

int Order::size() const noexcept {
    return (int)mTable.size();
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

void Order::insert(int before, OrderRow const& data) {
    assertCanInsert();
    checkIndex(before);
    mTable.insert(mTable.begin() + before, data);
}

void Order::insert(int before, OrderRow const *data, int count) {
    if (count <= 0) {
        throw std::invalid_argument("count must be > 0");
    }
    checkIndex(before);
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
void moveElement(Container &cont, int from, int to) {
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

void Order::move(int from, int to) {
    checkIndex(from);
    checkIndex(to);
    moveElement(mTable, from, to);
}

void Order::remove(int pattern, int count) {
    if (count > 0) {
        if (mTable.size() <= count) {
            throw std::runtime_error("cannot remove: Order must have at least 1 pattern");
        }
        checkIndex(pattern);

        auto iter = mTable.begin() + pattern;
        auto end = iter + count;
        if (iter > mTable.end()) {
            throw std::invalid_argument("too many orders to remove!");
        }
        mTable.erase(iter, end);
    } else {
        throw std::invalid_argument("count must be > 0");
    }
}

void Order::swapPatterns(int p1, int p2) {
    auto begin = mTable.begin();
    std::iter_swap(begin + p1, begin + p2);
}

void Order::setData(std::vector<OrderRow> const& data) {
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

void Order::assertCanSetData(std::vector<OrderRow> const& data) {
    auto sz = data.size();
    if (sz == 0) {
        throw std::invalid_argument("cannot set data: order must have at least 1 paterrn");
    } else if (sz > MAX_SIZE) {
        throw std::invalid_argument("cannot set data: too many patterns");
    }
}

void Order::checkIndex(int index) {
    if (index < 0 || index >= (int)mTable.size()) {
        throw std::invalid_argument("pattern index is invalid");
    }
}

}
