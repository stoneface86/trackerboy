
#include "trackerboy/data/DataItem.hpp"

namespace trackerboy {

DataItem::DataItem() :
    mId(0),
    mName()
{
}

DataItem::DataItem(const DataItem &item) :
    mId(item.mId),
    mName(item.mName)
{
}

DataItem::~DataItem() {

}

uint8_t DataItem::id() const noexcept {
    return mId;
}

std::string const& DataItem::name() const noexcept {
    return mName;
}

void DataItem::setId(uint8_t id) noexcept {
    mId = id;
}

void DataItem::setName(std::string const& name) noexcept {
    mName = name;
}


}
