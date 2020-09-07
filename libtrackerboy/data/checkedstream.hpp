
#pragma once

#define checkedRead(stream, buf, count) \
    do { \
        stream.read(reinterpret_cast<char*>(buf), count); \
        if (!stream.good()) { \
            return FormatError::none; \
        } \
    } while (false)

// wrapper for stream.write, will return writeError on failure
#define checkedWrite(stream, buf, count) \
    do {\
        stream.write(reinterpret_cast<const char*>(buf), count); \
        if (!stream.good()) { \
            return FormatError::none; \
        } \
    } while (false)
