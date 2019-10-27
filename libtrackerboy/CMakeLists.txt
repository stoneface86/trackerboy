
project("libtrackerboy")

set(SRC_FILES
	"instrument/Instrument.cpp"
	"instrument/InstrumentRuntime.cpp"
	"instrument/WaveTable.cpp"
	"pattern/Pattern.cpp"
	"synth/Channel.cpp"
	"synth/EnvChannel.cpp"
	"synth/gbsynth.cpp"
	"synth/Mixer.cpp"
	"synth/NoiseChannel.cpp"
	"synth/PulseChannel.cpp"
	"synth/sampletable.cpp"
	"synth/Sequencer.cpp"
	"synth/SweepPulseChannel.cpp"
	"synth/Synth.cpp"
	"synth/WaveChannel.cpp"
	"note.cpp"
)


add_library("trackerboy" STATIC ${SRC_FILES})

if (MSVC)
target_compile_options(
	trackerboy
	PRIVATE
	/W4
)
endif ()