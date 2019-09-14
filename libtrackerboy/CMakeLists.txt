
project("libtrackerboy")

set(INSTRUMENT_SRC
	"instrument/Instrument.cpp"
	"instrument/InstrumentRuntime.cpp"
	"instrument/NoiseInstrumentRuntime.cpp"
	"instrument/parse.cpp"
	"instrument/ParseError.cpp"
	"instrument/PulseInstrumentRuntime.cpp"
	"instrument/WaveInstrumentRuntime.cpp"
	"instrument/WaveTable.cpp"
	"note.cpp"
)

set(SYNTH_SRC
	"synth/Channel.cpp"
	"synth/EnvChannel.cpp"
	"synth/gbsynth.cpp"
	"synth/Mixer.cpp"
	"synth/NoiseChannel.cpp"
	"synth/PulseChannel.cpp"
	"synth/Sequencer.cpp"
	"synth/SweepPulseChannel.cpp"
	"synth/Synth.cpp"
	"synth/WaveChannel.cpp"
)

add_library("trackerboy" STATIC
	${INSTRUMENT_SRC}
	${SYNTH_SRC}
)

if (MSVC)
target_compile_options(
	trackerboy
	PRIVATE
	/W4
)
endif ()