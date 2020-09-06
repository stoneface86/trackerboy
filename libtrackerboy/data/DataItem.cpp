
#include "trackerboy/data/DataItem.hpp"

namespace trackerboy {

DataItem::DataItem() :
    mId(0),
    mName()
{
}

DataItem::~DataItem() {

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

bool DataItem::serialize(std::ostream &stream) noexcept {
    // id
    stream.write(reinterpret_cast<const char *>(&mId), 1);
    if (!stream.good()) {
        return false;
    }

    // name
    stream.write(mName.c_str(), mName.size() + 1);
    if (!stream.good()) {
        return false;
    }

    return serializeData(stream);
}

bool DataItem::deserialize(std::istream &stream) noexcept {
    stream.read(reinterpret_cast<char *>(&mId), 1);
    if (!stream.good()) {
        return false;
    }

    std::getline(stream, mName, '\0');
    if (!stream.good()) {
        return false;
    }

    return deserializeData(stream);
}


}
