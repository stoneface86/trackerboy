
#include <sstream>

#include "trackerboy/instrument.hpp"

namespace trackerboy {


ParseError::ParseError(size_t column, size_t lineno) :
    column(column),
    lineno(lineno),
    std::runtime_error("error occurred during parsing instrument")
{
}

size_t ParseError::getColumn() {
    return column;
}

size_t ParseError::getLineno() {
    return lineno;
}


}