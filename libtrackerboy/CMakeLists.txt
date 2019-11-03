
project("libtrackerboy")

set(SRC_FILES
	"instrument/Instrument.cpp"
	"instrument/InstrumentRuntime.cpp"
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

	set(PATTERN_TEST_SRC
		"pattern/test_Pattern.cpp"
	)

	add_catch_test(PATTERN_TEST_EXE pattern ${PATTERN_TEST_SRC})
	add_test(NAME Pattern COMMAND ${PATTERN_TEST_EXE} [pattern])

endif (TRACKERBOY_ENABLE_TESTS)
