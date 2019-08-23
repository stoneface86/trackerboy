project(ui CXX)

use_qt(Widgets Core)

set(INSTEDIT_SRC
	"instedit/instedit.cpp"
	"instedit/InstEditor.cpp"
	"instedit/Playback.cpp"
	"instedit/PaException.cpp"
	"instedit/SynthWorker.cpp"
)


add_executable(instedit ${GUI_TYPE} ${INSTEDIT_SRC})
target_link_libraries(instedit
	portaudio_static
	portaudio_ringbuffer
	trackerboy
	Qt5::Widgets
	Qt5::Core
)
