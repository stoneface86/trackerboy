
#pragma once

#include "core/PatternSelection.hpp"

#include "trackerboy/data/Pattern.hpp"

#include <QMimeData>

#include <memory>

//
// Container class for clipped pattern data. A PatternSelection can be used to
// store a copy of the pattern data within that selection. Once saved, the
// clip can be moved to a new location or restored to its original location.
// The clip can also be transferred to/from a QMimeData object, allowing it
// to be put on the system's clipboard.
//
class PatternClip {

public:

    //
    // MIME type for the clip data
    //
    static constexpr auto MIME_TYPE = "application/x-trackerboy-clip";

    //
    // Default contructor. Clip initialized with no data.
    //
    PatternClip();
    
    // copy constructor/assignment
    PatternClip(PatternClip const& clip);
    PatternClip& operator=(PatternClip const& clip);

    // move constructor/assignment
    PatternClip(PatternClip &&clip) noexcept;
    PatternClip& operator=(PatternClip &&clip) noexcept;

    ~PatternClip();


    //
    // Returns true if this object has a clip.
    //
    bool hasData() const;

    //
    // Gets the selection the clip was sourced from
    //
    PatternSelection const& selection();

    //
    // Restores previously clipped data to the given pattern.
    //
    void restore(trackerboy::Pattern &dest) const;

    //
    // Pastes this clip's data at the given position. If mix is true, then
    // paste data will be mixed with the destination pattern data, or only
    // empty cells in the destination will be overwritten by the clip data.
    //
    void paste(trackerboy::Pattern &dest, PatternCursor pos, bool mix) const;

    //
    // Saves pattern data from the given source pattern in the given selection.
    //
    void save(trackerboy::Pattern const& src, PatternSelection region);

    //
    // Transfers the clip data to the given QMimeData. Function does nothing
    // if the clip has no data. (Must call save first).
    //
    void toMime(QMimeData *data) const;

    //
    // Reads clip data from the given QMimeData. If successful, true is
    // returned. False is returned if the QMimeData doesn't have the required
    // format or the data was invalid.
    //
    bool fromMime(QMimeData const* data);

    friend bool operator==(PatternClip const& lhs, PatternClip const& rhs) noexcept;
    friend bool operator!=(PatternClip const& lhs, PatternClip const& rhs) noexcept;

private:

    void pasteImpl(trackerboy::Pattern &dest, std::optional<PatternCursor> pos, bool mixPaste) const;

    std::unique_ptr<char[]> mData;
    PatternSelection mLocation;


};
