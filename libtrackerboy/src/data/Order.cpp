
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
    if (before < 0 || before > (int)mTable.size()) {
        badIndex();
    }
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

OrderRow Order::nextUnused() const noexcept {

    OrderRow autoRow;

    for (size_t track = 0; track < 4; ++track) {
        
        std::array<char, 256> idmap{};
        for (auto const& row : mTable) {
            idmap[row[track]] = 1; // mark this id as used
        }

        auto begin = idmap.begin();
        auto end = idmap.end();
        auto iter = std::find(begin, end, 0); // find the first 0
        autoRow[track] = iter == end ? (uint8_t)0 : (uint8_t)(iter - begin);

    }
    return autoRow;
}

void Order::remove(int pattern, int count) {
    if (count > 0) {
        if ((int)mTable.size() <= count) {
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
        badIndex();
    }
}

void Order::badIndex() {
    throw std::invalid_argument("pattern index is invalid");
}

}
