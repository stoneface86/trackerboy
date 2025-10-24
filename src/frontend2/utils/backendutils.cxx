
#include "utils/backendutils.hxx"

#include <QStringEncoder>

QString toQString(BSlice const slice) {
    return QString::fromUtf8(slice.data, slice.len - 1);
}

BString toNimString(QString const &str) {
    auto nimStr = bInitString();
    QStringEncoder encoder(QStringConverter::Utf8);

    auto strIt = str.begin();
    char codePoint[4];
    for (auto i = str.size(); i > 0; --i) {
        auto strNext = strIt + 1;
        auto const codePointEnd =
            encoder.appendToBuffer(codePoint, {strIt, strNext});
        for (auto c = codePoint; c != codePointEnd; ++c) {
            bAdd(nimStr, *c);
        }

        strIt = strNext;
    }

    return {nimStr};
}
