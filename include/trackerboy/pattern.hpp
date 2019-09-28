
#pragma once

#include <array>
#include <cstdint>
using std::uint8_t;

#include "trackerboy/common.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {



struct TrackRow {
    Note note;
    uint8_t instrumentId;
	//
	// Flags
	// bit 0: Note set bit
	// bit 1: Instrument set bit
	// bit 2: effect set bit
	// bit 3: unused
	// bits 4-7: effect type
	//
	uint8_t flags;
	//
	// effect parameter(s)
	// bits 0-3: first parameter
	// bits 4-7: second parameter
	//
    uint8_t effect; // effect parameters
	
	enum RowFlags : uint8_t {
		FLAGS_NOTESET = 1,
		FLAGS_INSTSET = 2,
		FLAGS_EFFECTSET = 4,
		// bit 3 unused
		FLAGS_EFFECT = 0xF0
	};

	enum EffectType : uint8_t {
		EFFECT_SPEED        = 0x00,
		EFFECT_PATTERN_END  = 0x10,
		EFFECT_PITCH_UP	    = 0x20,
		EFFECT_PITCH_DOWN   = 0x30,
		EFFECT_VIBRATO      = 0x40,
		EFFECT_ARP          = 0x50,
		EFFECT_OFF          = 0x60,
		EFFECT_VOLUME       = 0x70,
		EFFECT_DUTY         = 0x80,
		EFFECT_FLAG         = 0x90
        // 10/16 effects used
	};

};


class Track {

public:

	Track();

	void clear(uint8_t row, TrackRow::RowFlags columns);

	void setNote(uint8_t row, Note note);

	void setInstrument(uint8_t row, uint8_t instrumentId);

	void setEffect( uint8_t row, TrackRow::EffectType effect, uint8_t param = 0);

	void move(ChType id, uint8_t rowStart, uint8_t rowEnd, uint8_t newrow);

private:

	std::array<TrackRow, 64> mTable;

};


class Pattern {

public:

    Pattern();

	Track& track(ChType id);

	

private:
    std::array<Track, 4> mTable;


};



}