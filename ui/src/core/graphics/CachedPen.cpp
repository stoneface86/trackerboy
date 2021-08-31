
#include "core/graphics/CachedPen.hpp"


CachedPen& CachedPen::operator=(QPen const& pen) {
    mPen = pen;
    return *this;
}

CachedPen& CachedPen::operator=(QPen &&pen) {
    mPen = std::move(pen);
    return *this;
}

QPen& CachedPen::get() {
    return mPen;
}
QPen const& CachedPen::get() const {
    return mPen;
}

QPen const& CachedPen::get(QColor const& color) {
    mPen.setColor(color);
    return mPen;
}
