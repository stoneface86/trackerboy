
#pragma once

#include <cstdint>
#include <string>
#include <ostream>
#include <istream>

namespace trackerboy {

class DataItem {

public:

    virtual ~DataItem();

    uint8_t id() const noexcept;

    std::string name() const noexcept;
    
    void setName(std::string name) noexcept;

    void setId(uint8_t id) noexcept;

    bool serialize(std::ostream &stream) noexcept;

    bool deserialize(std::istream &stream) noexcept;


protected:
    DataItem();

    virtual bool serializeData(std::ostream &stream) noexcept = 0;
    virtual bool deserializeData(std::istream &stream) noexcept = 0;
    
    uint8_t mId;
    std::string mName;

};

}
