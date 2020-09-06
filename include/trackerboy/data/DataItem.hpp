
#pragma once

#include <cstdint>
#include <string>
#include <fstream>

namespace trackerboy {

class DataItem {

public:

    virtual ~DataItem();

    uint8_t id() const noexcept;

    std::string name() const noexcept;
    
    void setName(std::string name) noexcept;

    void setId(uint8_t id) noexcept;

    bool serialize(std::ofstream &stream) noexcept;

    bool deserialize(std::ifstream &stream) noexcept;


protected:
    DataItem();

    virtual bool serializeData(std::ofstream &stream) noexcept = 0;
    virtual bool deserializeData(std::ifstream &stream) noexcept = 0;
    
    uint8_t mId;
    std::string mName;

};

}
