
#include "utils/backendutils.hxx"

QString toString(B::Slice slice) {
    return QString::fromUtf8(slice.data, slice.len);
}

Utf8Slice toUtf8(QString const& str) {
    Utf8Slice result;
    result.data = str.toUtf8();
    result.slice.len = result.data.size() + 1;
    result.slice.data =  result.data.data();
    return result;
}
