cmake_minimum_required(VERSION 3.5)
#set( CMAKE_VERBOSE_MAKEFILE on )

include(ExternalProject)
include("cmake/RustTriple.cmake")

project(mwc-qt-wallet)

set(Static_Build false CACHE BOOL "Build using static Qt and C++ runtime")


####################
#
# Qt configuration
#

if(Qt5_DIR STREQUAL "Qt5_DIR-NOTFOUND")
	unset(Qt5_DIR CACHE)
endif()

if(APPLE)
	set(Qt5_DIR "~/Qt/5.9/clang_64/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
elseif(LINUX)
	set(Qt5_DIR "/usr/local/opt/qt/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
elseif(WIN32)
	set(32bit false CACHE BOOL "Build for 32-bit Windows")
	if(Static_Build)
		if(32bit)
			set(Qt5_DIR "C:/Qt/5.9.8/win32-msvc-static/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
		else()
			set(Qt5_DIR "C:/Qt/5.9.8/win32-msvc_64-static/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
		endif()
	else()
		if(32bit)
			set(Qt5_DIR "C:/Qt/5.9.8/msvc2015/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
		else()
			set(Qt5_DIR "C:/Qt/5.9.8/msvc2015_64/lib/cmake/Qt5" CACHE PATH "The directory containing Qt5Config.cmake (i.e. .../lib/cmake/Qt5")
		endif()
	endif()
endif()
find_package(Qt5 COMPONENTS Core Gui Network Widgets REQUIRED)


####################
#
# Compiler options
#

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard level")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


####################
#
# Static build conf
#

if(Static_Build)
	message(${Qt5_DIR})
	if(MSVC)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
	endif()

	function(import_qt_libs)
		foreach(lib_base_name ${ARGN})
			add_library(${lib_base_name} STATIC IMPORTED)
			set_target_properties(${lib_base_name} PROPERTIES IMPORTED_LOCATION_DEBUG "${Qt5_DIR}/../../${lib_base_name}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
			set_target_properties(${lib_base_name} PROPERTIES IMPORTED_LOCATION_RELEASE "${Qt5_DIR}/../../${lib_base_name}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		endforeach()
	endfunction()

	import_qt_libs("preprocessor" "Qt5AccessibilitySupport" "Qt5EventDispatcherSupport" "Qt5FontDatabaseSupport" "Qt5ThemeSupport" "qtfreetype" "qtharfbuzz" "qtlibpng" "qtmain" "qtpcre2" "translator")

	set(COMMON_LINK_LIBRARIES preprocessor Qt5AccessibilitySupport Qt5EventDispatcherSupport Qt5FontDatabaseSupport Qt5ThemeSupport
		qtfreetype qtharfbuzz qtlibpng qtpcre2 qtmain translator)

	# TODO: other platforms
	if(WIN32)
		add_library(qwindows STATIC IMPORTED)
		set_target_properties(qwindows PROPERTIES IMPORTED_LOCATION_DEBUG "${Qt5_DIR}/../../../plugins/platforms/qwindowsd${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set_target_properties(qwindows PROPERTIES IMPORTED_LOCATION_RELEASE "${Qt5_DIR}/../../../plugins/platforms/qwindows${CMAKE_STATIC_LIBRARY_SUFFIX}")

		set(WIN32_LINK_LIBRARIES d3d9 dnsapi dwmapi dxguid imm32 iphlpapi mpr qwindows rpcrt4 shlwapi uxtheme version winmm ws2_32)
	endif()
endif()


####################
#
# Input files
#

file(GLOB SOURCE_FILES ./*.cpp ./control/*.cpp ./core/*.cpp ./state/*.cpp ./test/*.cpp ./tries/*.cpp ./util/*.cpp ./wallet/*.cpp ./wallet/tasks/*.cpp ./windows/*.cpp )
file(GLOB HEADER_FILES         ./control/*.h   ./core/*.h   ./state/*.h   ./test/*.h   ./tries/*.h   ./util/*.h   ./wallet/*.h   ./wallet/tasks/*.h   ./windows/*.h   )

# Generate necessary headers from .ui files. (qmake lets `uic` do this job.)
# hint from [Cross-platform Qt5 project using cmake](http://stackoverflow.com/questions/21174586/)
file(GLOB UI_SOURCES ./*/*.ui )
qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Handle .qrc files automatically
set(CMAKE_AUTORCC ON)


####################
#
# Outputs
#

add_executable(mwc-qt-wallet ${SOURCE_FILES} ${HEADER_FILES} ${UI_GENERATED_HEADERS} resources.qrc)
target_link_libraries(mwc-qt-wallet Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets
	Qt5::Gui_EGL Qt5::Gui_GLESv2 ${COMMON_LINK_LIBRARIES} ${WIN32_LINK_LIBRARIES})

if(WIN32)
	set(ICON_RC "icon.rc")
endif()

if(NOT Static_Build)
	# Copy Qt runtime libraries to build directory
	# [Copying Qt DLLs to executable directory on Windows using CMake](https://stackoverflow.com/questions/40564443/)
	add_custom_command(
		TARGET mwc-qt-wallet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:Qt5::Core>
			$<TARGET_FILE:Qt5::Gui>
			$<TARGET_FILE:Qt5::Network>
			$<TARGET_FILE:Qt5::Widgets>
			$<TARGET_FILE_DIR:mwc-qt-wallet>
	)
endif()


####################
#
# Project settings
#

if(MSVC)
	# Set Visual Studio start-up project
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mwc-qt-wallet)

	set(Debug_Console false CACHE BOOL "Build as a console application")
	if(NOT Debug_Console)
		# Build without a console
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	endif()
endif()


####################
#
# mwc713
#

rust_triple(TARGET_TRIPLE)
ExternalProject_Add(
	mwc713
	DOWNLOAD_COMMAND rustup install 1.35.0-${TARGET_TRIPLE}
	CONFIGURE_COMMAND rustup run 1.35.0-${TARGET_TRIPLE} cargo clean
	BUILD_COMMAND  rustup run 1.35.0-${TARGET_TRIPLE} cargo build $<$<NOT:$<CONFIG:Debug>>:--release> --target-dir ${CMAKE_CURRENT_BINARY_DIR}
	BINARY_DIR "${CMAKE_SOURCE_DIR}/backends/mwc713"
	INSTALL_COMMAND ""
	LOG_BUILD ON)
add_dependencies(mwc-qt-wallet mwc713)
