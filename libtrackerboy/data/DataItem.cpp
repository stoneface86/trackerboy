
#include "trackerboy/data/DataItem.hpp"

namespace trackerboy {

DataItem::DataItem() :
    mId(0),
    mName()
{
}

uint8_t DataItem::id() const noexcept {
    return mId;
}

std::string DataItem::name() const noexcept {
    return mName;
}

void DataItem::setId(uint8_t id) noexcept {
    mId = id;
}

void DataItem::setName(std::string name) noexcept {
    mName = name;
}


}
