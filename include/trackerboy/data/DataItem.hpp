
#pragma once

#include <cstdint>
#include <string>

namespace trackerboy {

class DataItem {

public:
    uint8_t id() const noexcept;

    std::string name() const noexcept;
    
    void setName(std::string name) noexcept;

    void setId(uint8_t id) noexcept;


protected:
    DataItem();
    
    uint8_t mId;
    std::string mName;

};

}
