
#pragma once

#include <QPen>

//
// Container class for a single, reusable QPen.
//
class CachedPen {

public:

    CachedPen& operator=(QPen const& pen);
    CachedPen& operator=(QPen &&pen);

    //
    // Access the cached QPen
    //
	QPen& get();
	QPen const& get() const;

    //
    // Gets the cached pen after setting the color.
    //
	QPen const& get(QColor const& color);

private:
	QPen mPen;

};
