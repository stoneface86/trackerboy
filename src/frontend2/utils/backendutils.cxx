
#include "utils/backendutils.hxx"

#include <QStringEncoder>

QString toQString(B::Slice slice) {
    return QString::fromUtf8(slice.data, slice.len - 1);
}

B::String toNimString(QString const &str) {
    auto nimstr = B::initString();
    QStringEncoder encoder(QStringConverter::Utf8);

    auto strIt = str.begin();
    char codePoint[4];
    for (auto i = str.size(); i > 0; --i) {
        auto strNext = strIt + 1;
        auto codePointEnd = encoder.appendToBuffer(codePoint, {strIt, strNext});
        for (auto c = codePoint; c != codePointEnd; ++c) {
            B::add(nimstr, *c);
        }

        strIt = strNext;
    }

    return {nimstr};
}
