
#pragma once

#include <cstdint>
#include <string>
#include <ostream>
#include <istream>

#include "trackerboy/fileformat.hpp"

namespace trackerboy {

class DataItem {

public:

    virtual ~DataItem();

    uint8_t id() const noexcept;

    std::string name() const noexcept;
    
    void setName(std::string name) noexcept;

    void setId(uint8_t id) noexcept;

    FormatError serialize(std::ostream &stream) noexcept;

    FormatError deserialize(std::istream &stream) noexcept;


protected:
    DataItem();
    DataItem(const DataItem &item);

    virtual FormatError serializeData(std::ostream &stream) noexcept = 0;
    virtual FormatError deserializeData(std::istream &stream) noexcept = 0;

    uint8_t mId;
    std::string mName;

};

}
