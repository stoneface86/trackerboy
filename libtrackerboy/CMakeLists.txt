
project("libtrackerboy")

set(SRC_FILES
	"instrument/Instrument.cpp"
	"instrument/InstrumentRuntime.cpp"
	"pattern/Pattern.cpp"
	"pattern/PatternRuntime.cpp"
	"pattern/TrackRuntime.cpp"
	"pattern/Track.cpp"
	"song/Order.cpp"
	"song/Song.cpp"
	"song/SongRuntime.cpp"
	"synth/Channel.cpp"
	"synth/EnvChannel.cpp"
	"synth/gbsynth.cpp"
	"synth/Mixer.cpp"
	"synth/NoiseChannel.cpp"
	"synth/Osc.cpp"
	"synth/PulseChannel.cpp"
	"synth/sampletable.cpp"
	"synth/Sequencer.cpp"
	"synth/SweepPulseChannel.cpp"
	"synth/Synth.cpp"
	"synth/WaveChannel.cpp"
	"synth/Waveform.cpp"
	"fileformat.cpp"
	"Module.cpp"
	"note.cpp"
	"Table.cpp"
)


add_library("trackerboy" STATIC ${SRC_FILES})

if (MSVC)
target_compile_options(
	trackerboy
	PRIVATE
	/W4
)
endif ()

if (TRACKERBOY_ENABLE_TESTS)

	set(TEST_SRC_FILES
		"pattern/test_Pattern.cpp"
		"song/test_Order.cpp"
	)

	add_catch_test(TEST_EXE ${TEST_SRC_FILES})
	add_test(NAME Pattern COMMAND ${TEST_EXE} [pattern])
	add_test(NAME Order COMMAND ${TEST_EXE} [order])

endif (TRACKERBOY_ENABLE_TESTS)
