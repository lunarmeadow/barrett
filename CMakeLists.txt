cmake_minimum_required(VERSION 3.10)
project(barrett
	DESCRIPTION "A Rise of the Triad source port tailored towards moddability and QoL."
	HOMEPAGE_URL "https://github.com/lunarmeadow/barrett"
	LANGUAGES C CXX
)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# only ONE of these may be active at a time (or none for the default registered version)
option(BARRETT_SHAREWARE "Build Shareware version" OFF)
option(BARRETT_SUPERROTT "Build SuperROTT version" OFF)
option(BARRETT_SITELICENSE "Build Site License version" OFF)
option(BARRETT_USEASAN "Turn on ASAN" OFF)

if((BARRETT_SHAREWARE AND BARRETT_SUPERROTT) OR (BARRETT_SUPERROTT AND BARRETT_SITELICENSE) OR (BARRETT_SHAREWARE AND BARRETT_SITELICENSE))
	message(FATAL_ERROR "Only one build option may be active at a time! (or none for the registered version)")
endif()

if(BARRETT_SHAREWARE)
	message(STATUS "Building Shareware version")
elseif(BARRETT_SUPERROTT)
	message(STATUS "Building SuperROTT version")
elseif(BARRETT_SITELICENSE)
	message(STATUS "Building Site License version")
else()
	message(STATUS "Building Registered version")
endif()

cmake_policy(SET CMP0135 NEW)

include(FetchContent)

FetchContent_Declare(
	adlmidi
	GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI.git
	GIT_TAG v1.6.1
	EXCLUDE_FROM_ALL
)
FetchContent_Declare(
	inih
	GIT_REPOSITORY https://github.com/benhoyt/inih.git
	GIT_TAG r62
	EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(adlmidi inih)

if(MSVC)
	set(BARRETT_SDL2_VERSION "2.32.10")
	FetchContent_Declare(
		sdl2_msvc
		URL "https://github.com/libsdl-org/SDL/releases/download/release-${BARRETT_SDL2_VERSION}/SDL2-devel-${BARRETT_SDL2_VERSION}-VC.zip"
		URL_HASH MD5=a7f901f6f9630f37d9c8bcf8e57ba02a
	)
	FetchContent_MakeAvailable(sdl2_msvc)
	set(SDL2_DIR "${sdl2_msvc_SOURCE_DIR}/cmake")
	set(BARRETT_SDL2_MIXER_VERSION "2.8.1")
	FetchContent_Declare(
		sdl2_mixer_msvc
		URL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${BARRETT_SDL2_MIXER_VERSION}/SDL2_mixer-devel-${BARRETT_SDL2_MIXER_VERSION}-VC.zip"
		URL_HASH MD5=5922d1f423b266bd0df7dd90f5dd6e4b
	)
	FetchContent_MakeAvailable(sdl2_mixer_msvc)
	set(SDL2_mixer_DIR "${sdl2_mixer_msvc_SOURCE_DIR}/cmake")
endif()

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(SDL2_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL2_mixer)

set(AUDIOLIB_SOURCES
	${PROJECT_SOURCE_DIR}/src/audiolib/fx_man.c
	${PROJECT_SOURCE_DIR}/src/audiolib/dsl.c
	${PROJECT_SOURCE_DIR}/src/audiolib/ll_man.c
	${PROJECT_SOURCE_DIR}/src/audiolib/multivoc.c
	${PROJECT_SOURCE_DIR}/src/audiolib/mv_mix.c
	${PROJECT_SOURCE_DIR}/src/audiolib/mvreverb.c
	${PROJECT_SOURCE_DIR}/src/audiolib/nodpmi.c
	${PROJECT_SOURCE_DIR}/src/audiolib/pitch.c
	${PROJECT_SOURCE_DIR}/src/audiolib/user.c
	${PROJECT_SOURCE_DIR}/src/audiolib/usrhooks.c
)

add_library(audiolib STATIC ${AUDIOLIB_SOURCES})

add_library(ASAN::ASAN IMPORTED INTERFACE)
target_compile_options(ASAN::ASAN INTERFACE -fsanitize=address,undefined $<$<NOT:$<COMPILE_LANG_AND_ID:C,MSVC>>:-fno-omit-frame-pointer>)
target_link_options(ASAN::ASAN INTERFACE -fsanitize=address,undefined)

set_target_properties(audiolib PROPERTIES C_STANDARD_REQUIRED ON C_STANDARD 23)
target_link_libraries(audiolib PRIVATE SDL2::SDL2 SDL2_mixer::SDL2_mixer)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(audiolib PRIVATE -Wall -Wextra -Wpedantic -Wfatal-errors)
endif()

set(BARRETT_SOURCES
	${PROJECT_SOURCE_DIR}/src/cin_actr.c
	${PROJECT_SOURCE_DIR}/src/cin_efct.c
	${PROJECT_SOURCE_DIR}/src/cin_evnt.c
	${PROJECT_SOURCE_DIR}/src/cin_glob.c
	${PROJECT_SOURCE_DIR}/src/cin_main.c
	${PROJECT_SOURCE_DIR}/src/cin_util.c
	${PROJECT_SOURCE_DIR}/src/dosutil.c
	${PROJECT_SOURCE_DIR}/src/engine.c
	${PROJECT_SOURCE_DIR}/src/isr.c
	${PROJECT_SOURCE_DIR}/src/modexlib.c
	${PROJECT_SOURCE_DIR}/src/rt_actor.c
	${PROJECT_SOURCE_DIR}/src/rt_battl.c
	${PROJECT_SOURCE_DIR}/src/rt_build.c
	${PROJECT_SOURCE_DIR}/src/rt_cfg.c
	${PROJECT_SOURCE_DIR}/src/rt_crc.c
	${PROJECT_SOURCE_DIR}/src/rt_com.c
	${PROJECT_SOURCE_DIR}/src/rt_debug.c
	${PROJECT_SOURCE_DIR}/src/rt_dmand.c
	${PROJECT_SOURCE_DIR}/src/rt_door.c
	${PROJECT_SOURCE_DIR}/src/rt_draw.c
	${PROJECT_SOURCE_DIR}/src/rt_fixed.c
	${PROJECT_SOURCE_DIR}/src/rt_floor.c
	${PROJECT_SOURCE_DIR}/src/rt_game.c
	${PROJECT_SOURCE_DIR}/src/rt_in.c
	${PROJECT_SOURCE_DIR}/src/rt_main.c
	${PROJECT_SOURCE_DIR}/src/rt_map.c
	${PROJECT_SOURCE_DIR}/src/rt_menu.c
	${PROJECT_SOURCE_DIR}/src/rt_msg.c
	${PROJECT_SOURCE_DIR}/src/rt_net.c
	${PROJECT_SOURCE_DIR}/src/rt_playr.c
	${PROJECT_SOURCE_DIR}/src/rt_rand.c
	${PROJECT_SOURCE_DIR}/src/rt_scale.c
	${PROJECT_SOURCE_DIR}/src/rt_sound.c
	${PROJECT_SOURCE_DIR}/src/rt_stat.c
	${PROJECT_SOURCE_DIR}/src/rt_state.c
	${PROJECT_SOURCE_DIR}/src/rt_str.c
	${PROJECT_SOURCE_DIR}/src/rt_ted.c
	${PROJECT_SOURCE_DIR}/src/rt_util.c
	${PROJECT_SOURCE_DIR}/src/rt_view.c
	${PROJECT_SOURCE_DIR}/src/rt_vid.c
	${PROJECT_SOURCE_DIR}/src/rt_err.c
	${PROJECT_SOURCE_DIR}/src/scriplib.c
	${PROJECT_SOURCE_DIR}/src/w_wad.c
	${PROJECT_SOURCE_DIR}/src/z_zone.c
	${PROJECT_SOURCE_DIR}/src/byteordr.c
	${PROJECT_SOURCE_DIR}/src/dukemusc.c
	${PROJECT_SOURCE_DIR}/src/winrott.c
	${PROJECT_SOURCE_DIR}/src/queue.c
	${PROJECT_SOURCE_DIR}/src/HashTable.c
	${PROJECT_SOURCE_DIR}/src/opl.c
	${inih_SOURCE_DIR}/ini.c
)

add_executable(barrett ${BARRETT_SOURCES})
target_include_directories(barrett PRIVATE ${inih_SOURCE_DIR})
target_link_libraries(barrett PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> SDL2::SDL2 SDL2_mixer::SDL2_mixer ADLMIDI)
target_link_libraries(barrett PRIVATE audiolib)
target_link_libraries(barrett PUBLIC $<$<BOOL:${BARRETT_USEASAN}>:ASAN::ASAN>)
target_compile_definitions(barrett PRIVATE SHAREWARE=$<BOOL:${BARRETT_SHAREWARE}>)
target_compile_definitions(barrett PRIVATE SUPERROTT=$<BOOL:${BARRETT_SUPERROTT}>)
target_compile_definitions(barrett PRIVATE SITELICENSE=$<BOOL:${BARRETT_SITELICENSE}>)
if(MSVC)
	target_compile_definitions(barrett PRIVATE PLATFORM_WIN32=1)
	target_compile_definitions(barrett PRIVATE PLATFORM_UNIX=0)
	target_compile_options(barrett PRIVATE /wd4996 /wd4305 /wd4244 /wd4101)
else()
	target_compile_definitions(barrett PRIVATE PLATFORM_WIN32=0)
	target_compile_definitions(barrett PRIVATE PLATFORM_UNIX=1)
endif()
set_target_properties(barrett PROPERTIES C_STANDARD_REQUIRED ON C_STANDARD 23)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(barrett PRIVATE -Wall -Wextra -Wshadow=compatible-local -Wpedantic -Wfatal-errors)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(barrett PRIVATE -g -O0)
		if(NOT MINGW)
			target_link_options(barrett PRIVATE -rdynamic)
		endif()
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_compile_options(barrett PRIVATE -O2)
	endif()
endif()

# statically link libc on windows bc mingw is a royal pain in the ass.
# maybe use MSVC eventually but this should be good for now.
# windows executable is probably huge so it should be stripped and compiled for size.
if(MINGW)
	target_compile_options(barrett PRIVATE -static-libgcc -static-libstdc++ -static)
	target_link_options(barrett PRIVATE -static-libgcc -static-libstdc++ -static)
endif()
