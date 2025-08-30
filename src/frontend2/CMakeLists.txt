
project(frontend2 CXX)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

if (WIN32)
  set(GUI_TYPE WIN32)
  set(WINDOWS_APPICON_RC "resources/img/app.rc")
elseif (APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
  set(MACOSX_APPICON_ICNS "resources/img/app.icns")
else ()
  set(GUI_TYPE "")
endif ()

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("version.in.hxx" "${CMAKE_CURRENT_BINARY_DIR}/version.hxx" @ONLY)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(frontendObj OBJECT
  "core/ColorTheme.cxx"
  "core/Document.cxx"
  "core/icons.cxx"
  "core/settings.cxx"
  "forms/CommentsDialog.cxx"
  "forms/MainWindow.cxx"
  "forms/ModulePropertiesDialog.cxx"
  "forms/PersistantDialog.cxx"
  "forms/SongListEditor.cxx"
  "model/NameListModel.cxx"
  "model/SongListEditorModel.cxx"
  "model/SongModel.cxx"
  "model/TableModel.cxx"
  "resources/resources.qrc"
  "utils/actions.cxx"
  "utils/backendutils.cxx"
  "utils/RecentFiles.cxx"
  "utils/ToolbarLayout.cxx"
  "utils/string.cxx"
  "widgets/Sidebar/SongSettingsWidget.cxx"
  "widgets/CustomSpinBox.cxx"
  "widgets/EnvelopeForm.cxx"
  "widgets/PianoWidget.cxx"
  "widgets/Sidebar.cxx"
  "widgets/TickrateForm.cxx"
)
target_link_libraries(frontendObj PUBLIC Qt6::Widgets backend)
target_compile_features(frontendObj PUBLIC cxx_std_17)
if (MSVC)
  target_compile_options(frontendObj PUBLIC
    /W4
  )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # -Wno-missing-field-initializers prevents warnings when zero-initializing a struct
  target_compile_options(frontendObj PUBLIC
    -Wall
    -Wextra
    -pedantic
    -Wno-missing-field-initializers
  )

  # remove any unused code/data when building
  if (NOT ${DEBUG_BUILD})
    target_compile_options(frontendObj PUBLIC
      -ffunction-sections -fdata-sections -Wl,--gc-sections
    )
  endif ()
endif ()

if (NOT ${DEBUG_BUILD})
  target_compile_definitions(frontendObj PUBLIC QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
endif ()

if (ENABLE_UNITY AND ${CMAKE_VERSION} VERSION_GREATER "3.15")
  set_target_properties(frontendObj PROPERTIES UNITY_BUILD ON)
endif ()

if (APPLE)
  set_source_files_properties(${MACOSX_APPICON_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif ()

add_executable(
  frontend ${GUI_TYPE}
  "main.cxx"
  $<TARGET_OBJECTS:frontendObj>
  "${WINDOWS_APPICON_RC}"
  "${MACOSX_APPICON_ICNS}"
)
target_link_libraries(frontend PRIVATE frontendObj)
set_target_properties(frontend PROPERTIES OUTPUT_NAME "trackerboy")

if (BUILD_TESTING)
  add_executable(
    testSongListModel ${GUI_TYPE}
    "test/testSongListModel.cxx"
    $<TARGET_OBJECTS:frontendObj>
  )
  target_link_libraries(testSongListModel PRIVATE frontendObj)

  add_executable(
    testPianoWidget ${GUI_TYPE}
    "test/testPianoWidget.cxx"
    $<TARGET_OBJECTS:frontendObj>
  )
  target_link_libraries(testPianoWidget PRIVATE frontendObj)
endif ()

add_custom_target(
  runFrontend
  COMMAND frontend
  COMMENT "Running frontend"
)
