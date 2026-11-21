# Name:        CMakeLists.txt
# Description: build of The Simpsons: the game (from its assets, in a window or headless) and its development tools
# Version:     0.1.0
# Created:     05-11-2026
# Modified:    13-11-2026
# Author:      Sebastian Toyos
cmake_minimum_required(VERSION 3.20)
project(simpsons VERSION 1.0.3 LANGUAGES C CXX)    # (the port's release: The Simpsons' OPTIONS shows it)

# A generator of one configuration (Makefiles, Ninja: Linux's and macOS's by default) given no build type builds the
# game optimized, as the packages are (outrunners-port's: without, it is slower)
get_property(SIMPSONS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT SIMPSONS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "The build type: Release unless given" FORCE)
endif()

# The testing builds' suffix to the version The Simpsons' OPTIONS shows (-TEST and the CI run's number), from the
# environment's SIMPSONS_VERSION_SUFFIX unless given
set(SIMPSONS_VERSION_SUFFIX "$ENV{SIMPSONS_VERSION_SUFFIX}" CACHE STRING "The suffix of the version OPTIONS shows")

# The packages a player downloads carry SDL2 inside the games, built from its own source (tmnt-port's): the package is
# the executables alone, no library beside them. Its source: SIMPSONS_SDL2_SOURCE where this computer has one (a
# copy of SDL's release-2.32.10, build/downloads/SDL2-src), else SDL's repository at that tag
option(SIMPSONS_STATIC_SDL2 "Build SDL2 from its source into the executables" OFF)
set(SIMPSONS_SDL2_SOURCE ${CMAKE_SOURCE_DIR}/build/downloads/SDL2-src CACHE PATH "SDL2's source (release-2.32.10)")

# The devices (ports/PLAN.md, D), as tmnt-port's: the Nintendo Switch with devkitPro's toolchain
# (aarch64-none-elf-cmake: ports/switch/switch.cmake), Anbernic's H700 handhelds on their stock firmware with the
# toolchain ports/anbernic/zig_aarch64.cmake (ports/anbernic/anbernic.cmake), Android with the NDK's toolchain
# (ports/android/android.cmake). A device builds the two games alone (its own targets: simpsons_switch_games,
# simpsons_anbernic_games, simpsons_apks), from the assets pack a maker put in or the one it makes from the player's
# ROM set on the device; no window of the first run, no makers
if(NINTENDO_SWITCH OR CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
	set(SIMPSONS_SWITCH ON)
	set(SIMPSONS_DEVICE ON)
elseif(ANDROID)
	set(SIMPSONS_ANDROID ON)
	set(SIMPSONS_DEVICE ON)
endif()
option(SIMPSONS_ANBERNIC "Build for Anbernic's H700 handhelds (ports/anbernic/anbernic.cmake)" OFF)
if(SIMPSONS_ANBERNIC)
	set(SIMPSONS_DEVICE ON)
endif()
# iOS and iPadOS: Xcode's toolchain through CMake's Xcode generator (CMAKE_SYSTEM_NAME iOS: ports/ios/ios.cmake, its
# target simpsons_ios_games)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
	set(SIMPSONS_IOS ON)
	set(SIMPSONS_DEVICE ON)
endif()
# The Xbox in Developer Mode (and Windows 10 and 11): UWP apps, Visual Studio's generator with CMAKE_SYSTEM_NAME
# WindowsStore (ports/uwp/uwp.cmake, its target simpsons_uwp_games). Every library of the game built for an app's
# container (WINAPI_FAMILY_APP: SDL's headers then say __WINRT__)
if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	set(SIMPSONS_UWP ON)
	set(SIMPSONS_DEVICE ON)
	add_compile_definitions(WINAPI_FAMILY=WINAPI_FAMILY_APP)
	# (The Simpsons' LAN game in an app: arcade-lan has not been built for one yet)
	option(SIMPSONS_UWP_LAN "Build The Simpsons' LAN game into the UWP apps" OFF)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Every executable lands in build/bin (SIMPSONS_BIN_DIR), whatever the generator or configuration
set(SIMPSONS_BIN_DIR ${CMAKE_SOURCE_DIR}/build/bin CACHE PATH "Where the executables go")
foreach(config IN ITEMS "" _DEBUG _RELEASE _RELWITHDEBINFO _MINSIZEREL)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${config} ${SIMPSONS_BIN_DIR})
endforeach()

if(MSVC)
	add_compile_options(/W3 /utf-8)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	# with SIMPSONS_STATIC_SDL2 the C runtime goes inside the executables too: they need no Visual C++
	# redistributable on the player's machine
	if(SIMPSONS_STATIC_SDL2)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)    # (SDL2's targets too)
	endif()
endif()

# The game's executables nest their calls as the program's do (a jump to another function is a call that returns,
# and the translated code's locals are many): their main thread gets a stack of 64 MB, which on Windows the
# executable sets at link time: /STACK for MSVC and clang-cl, through the compiler's driver for Clang aimed at MSVC's
# runtime, --stack for MinGW's GCC and Clang (ld, lld). On Linux and macOS the main thread's stack is the system's,
# not the executable's: the limit the game is started under (`ulimit -s`, 8 MB by default on most systems; macOS's
# linker can also write one into the executable, -stack_size), and nothing here asks for one. The game has been run
# with 64 MB only.
function(simpsons_game_stack target)
	if(MSVC)
		target_link_options(${target} PRIVATE /STACK:67108864)
	elseif(WIN32 AND CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
		target_link_options(${target} PRIVATE LINKER:/STACK:67108864)
	elseif(WIN32)
		target_link_options(${target} PRIVATE LINKER:--stack,67108864)
	endif()
endfunction()

# The board as the program sees it (memory map, banks, interrupts, the EEPROM), the controls' scripts, the ROM set's
# regions, and a save state's file and parts (runtime/src/state.c, simpsons/state.h: each part visits its own)
add_library(simpsons_runtime STATIC
	runtime/src/machine.c
	runtime/src/sound.c
	runtime/src/script.c
	runtime/src/game_data.c
	runtime/src/state.c
)
target_include_directories(simpsons_runtime PUBLIC runtime/include PRIVATE runtime/src)

# zlib's inflate (the ROM set's .zip, the assets' PNG files)
add_library(simpsons_inflate STATIC runtime/src/inflate.c)
target_include_directories(simpsons_inflate PUBLIC runtime/src)

# The reader of the player's ROM set: for the tools that run the original code, and for the game's first run, which
# takes from it only what the assets give (the assets pack's block below)
add_library(simpsons_rom_set STATIC runtime/src/rom_set.c)
target_link_libraries(simpsons_rom_set PUBLIC simpsons_runtime simpsons_inflate)
target_include_directories(simpsons_rom_set PRIVATE runtime/src)

# The game's own video (runtime/src/video.c, simpsons/video.h): the frames MAME's video draws, pixel for pixel
# (kref --video-check)
add_library(simpsons_video STATIC runtime/src/video.c)
target_link_libraries(simpsons_video PUBLIC simpsons_runtime)

# The game's own sound chips (runtime/src/ym2151.c, k053260.c): the samples MAME's make (kref --sound-check)
add_library(simpsons_sound_chips STATIC runtime/src/ym2151.c runtime/src/k053260.c)
target_link_libraries(simpsons_sound_chips PUBLIC simpsons_runtime)

# The game's assets (runtime/src/assets.c, png.c, game/assets.h): the ROM set's bytes back from the files
# tools/extract_assets.py writes (kref --assets-check)
add_library(simpsons_assets STATIC runtime/src/assets.c runtime/src/png.c runtime/src/music.c)
target_link_libraries(simpsons_assets PUBLIC simpsons_runtime simpsons_inflate)

# The game's own sound board: the Z80 sound program decompiled to C (src/sound/, tools/decompile_sound.py) on its
# timing model and the board's chips (runtime/src/sound_cpu.c, sound_board.c, simpsons/sound_board.h): the samples and
# the RAM MAME's board makes under the same timing model (kref --sound-timing --sound-board-check)
file(GLOB SIMPSONS_SOUND_PROGRAM CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/sound/*.c)
add_library(simpsons_sound STATIC runtime/src/sound_cpu.c runtime/src/sound_board.c ${SIMPSONS_SOUND_PROGRAM})
target_include_directories(simpsons_sound PUBLIC runtime/include PRIVATE runtime/src src/sound)
target_link_libraries(simpsons_sound PUBLIC simpsons_sound_chips simpsons_runtime)
if(MSVC)
	target_compile_options(simpsons_sound PRIVATE /wd4102 /wd4244)
endif()

find_package(Python3 COMPONENTS Interpreter)

# The reference tools: the original program on MAME's cores (tools/core/konami/reference): Konami's CPU, its
# disassembler's oracle and kref on the board model; the video and the sound board follow
option(SIMPSONS_REFERENCE_TOOLS "Build the MAME-based reference tools" ON)
if(SIMPSONS_REFERENCE_TOOLS)
	if(NOT Python3_FOUND)
		message(FATAL_ERROR "The reference tools need Python 3 (MAME's Konami CPU core is written by m6809make.py)")
	endif()
	set(K_CPU 053248)
	set(K_GAME_RUNTIME simpsons_rom_set simpsons_video simpsons_sound_chips simpsons_assets)
	set(K_GAME_SOUND simpsons_sound)
	include(${CMAKE_SOURCE_DIR}/tools/core/konami/reference/reference.cmake)
	# the reference's sound board runs the game's own chips beside MAME's (kref --sound-check), wherever it goes
	target_link_libraries(ksound_053260_reference PUBLIC simpsons_sound_chips)
	# the translated program's helpers on all their inputs (tools/core/konami/reference/compare_konami_ops.py)
	add_executable(konami_ops_check tools/core/konami/reference/konami_ops_check.c runtime/src/konami_ops.c)
	target_include_directories(konami_ops_check PRIVATE runtime/include)
endif()

# The game: the decompiled 053248 program (src/, tools/decompile.py; SIMPSONS_DECOMPILED_DIR picks another copy) on
# its runtime (runtime/src/game_program.c: the frame, the bank, the interrupt mask and the interrupts, the tables it
# reads by name from the assets), the board's code in place of the boot's ROM sum (runtime/src/board_game.c), the
# game's own video and sound board, from its assets (runtime/src/game.c). simpsons_game_headless runs it with no
# screen or sound (check_scenarios.py compares it with kref), simpsons in a window. It opens no ROM
set(SIMPSONS_DECOMPILED_DIR ${CMAKE_SOURCE_DIR}/src CACHE PATH "The game's C (tools/decompile.py)")
file(GLOB_RECURSE SIMPSONS_DECOMPILED CONFIGURE_DEPENDS ${SIMPSONS_DECOMPILED_DIR}/*.c)
list(FILTER SIMPSONS_DECOMPILED EXCLUDE REGEX "/sound/[^/]*\\.c$")    # (the sound program's: simpsons_sound)
if(SIMPSONS_DECOMPILED)
	add_library(simpsons_game_program STATIC runtime/src/game_program.c runtime/src/konami_ops.c
		runtime/src/board_game.c ${SIMPSONS_DECOMPILED})
	target_include_directories(simpsons_game_program PUBLIC runtime/include PRIVATE ${SIMPSONS_DECOMPILED_DIR})
	target_link_libraries(simpsons_game_program PUBLIC simpsons_runtime)
	if(MSVC)
		target_compile_options(simpsons_game_program PRIVATE /bigobj /wd4244 /wd4146 /wd4102)
	else()
		target_compile_options(simpsons_game_program PRIVATE -fno-strict-aliasing)
	endif()
	add_library(simpsons_game_decompiled STATIC runtime/src/game.c)
	target_link_libraries(simpsons_game_decompiled PUBLIC simpsons_game_program simpsons_video simpsons_sound
		simpsons_assets)
	add_executable(simpsons_game_headless ports/headless/main.c)
	target_link_libraries(simpsons_game_headless PRIVATE simpsons_game_decompiled)
	# (the decompiled calls nest as the program's do; a jump to another function is a call that returns)
	simpsons_game_stack(simpsons_game_headless)
	set(SIMPSONS_WINDOW_PROGRAM simpsons_game_decompiled)
else()
	message(STATUS "No decompiled program in src/ (tools/decompile.py): no simpsons_game_headless")
endif()

# The game: the 053248 program translated to C an instruction at a time (the translation scaffold: tools/recompile.py
# writes build/generated, from the player's ROM: never part of the repository) with the board's own code in place of
# its ROM test, on the board model with frame timing, the game's own video and sound board, from its assets
# (runtime/src/game.c). simpsons_headless runs it with no screen or sound (check_scenarios.py --translated compares it
# with kref), simpsons in a window (SDL2). Neither opens the ROM set (tools/rom_in_executables.py: they hold none of it).
file(GLOB SIMPSONS_GENERATED CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/build/generated/*.c)
if(SIMPSONS_GENERATED)
	add_library(simpsons_program STATIC runtime/src/konami_cpu.c runtime/src/konami_ops.c runtime/src/board_code.c
		${SIMPSONS_GENERATED})
	target_include_directories(simpsons_program PUBLIC runtime/include PRIVATE ${CMAKE_SOURCE_DIR}/build/generated)
	target_link_libraries(simpsons_program PUBLIC simpsons_runtime)
	add_library(simpsons_game STATIC runtime/src/game.c)
	target_link_libraries(simpsons_game PUBLIC simpsons_program simpsons_video simpsons_sound simpsons_assets)
	add_executable(simpsons_headless ports/headless/main.c)
	target_link_libraries(simpsons_headless PRIVATE simpsons_game)
	if(MSVC)
		target_compile_options(simpsons_program PRIVATE /bigobj /wd4102 /wd4244)
	endif()
	# (the translated calls nest as the program's do, and the code's locals are many)
	simpsons_game_stack(simpsons_headless)
	if(NOT SIMPSONS_WINDOW_PROGRAM)
		set(SIMPSONS_WINDOW_PROGRAM simpsons_game)
	endif()
else()
	message(STATUS "No generated program in build/generated (tools/recompile.py): no game")
endif()

# The assets pack (simpsons.assets, runtime/src/assets_pack.c): what the assets load as one file, for the devices; and
# the same load from the player's ROM set straight, through the assets' layout (runtime/src/assets_layout.c: places
# and lengths only, which simpsons_assets_check --write-layout writes from the assets folder), so the ROM set's reader
# is the game's first run too. The pack's version is the layout's and the names and places of the tables both
# programs' C reads by name: the decompiled program's registry (src/table.c) and the sound program's, the sound
# program's alone for the translation scaffold. simpsons_assets_check (ports/headless/assets_check.c) checks that the
# folder's pack and the ROM set's are the same, byte for byte. Each game's library (runtime/src/game.c: game_load takes
# a folder or a pack, game_load_rom_set the ROM set) links the pack of its own program
function(simpsons_add_assets_pack target program)
	add_library(${target} STATIC runtime/src/assets_pack.c runtime/src/assets_layout.c)
	target_include_directories(${target} PRIVATE runtime/src)
	target_link_libraries(${target} PUBLIC simpsons_assets simpsons_rom_set simpsons_sound ${program})
endfunction()
if(TARGET simpsons_game_decompiled)
	simpsons_add_assets_pack(simpsons_assets_pack simpsons_game_program)
	target_link_libraries(simpsons_game_decompiled PUBLIC simpsons_assets_pack)
	target_link_libraries(simpsons_game_headless PRIVATE simpsons_assets_pack)
	add_executable(simpsons_assets_check ports/headless/assets_check.c)
	target_link_libraries(simpsons_assets_check PRIVATE simpsons_assets_pack simpsons_game_decompiled)
endif()
if(TARGET simpsons_game)
	simpsons_add_assets_pack(simpsons_assets_pack_scaffold "")
	target_compile_definitions(simpsons_assets_pack_scaffold PRIVATE SIMPSONS_TRANSLATION_SCAFFOLD)
	target_link_libraries(simpsons_game PUBLIC simpsons_assets_pack_scaffold)
	target_link_libraries(simpsons_headless PRIVATE simpsons_assets_pack_scaffold)
endif()

# The console version (The Simpsons, console/PLAN.md): the same decompiled game with the console's own versions of
# the functions console/replaced.txt lists (console/*.c, SIMPSONS_CONSOLE: their originals are the decompiled C's
# CONSOLE_ORIGINAL definitions, original_name there), its menus written with the game's own letters. Its OPTIONS show
# the version (SIMPSONS_VERSION: the project's, and SIMPSONS_VERSION_SUFFIX)
file(GLOB SIMPSONS_CONSOLE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/console/*.c)
if(SIMPSONS_DECOMPILED AND SIMPSONS_CONSOLE_SOURCES)
	# (and its save states, from the pause: runtime/src/game_state.c, game/state.h)
	add_library(simpsons_console_program STATIC runtime/src/game_program.c runtime/src/konami_ops.c
		runtime/src/board_game.c runtime/src/game_state.c ${SIMPSONS_DECOMPILED} ${SIMPSONS_CONSOLE_SOURCES})
	target_include_directories(simpsons_console_program PUBLIC runtime/include console PRIVATE ${SIMPSONS_DECOMPILED_DIR})
	target_compile_definitions(simpsons_console_program PUBLIC SIMPSONS_CONSOLE
		SIMPSONS_VERSION="${PROJECT_VERSION}${SIMPSONS_VERSION_SUFFIX}")
	target_link_libraries(simpsons_console_program PUBLIC simpsons_runtime)
	if(MSVC)
		target_compile_options(simpsons_console_program PRIVATE /bigobj /wd4244 /wd4146 /wd4102)
	else()
		target_compile_options(simpsons_console_program PRIVATE -fno-strict-aliasing)
	endif()
	add_library(simpsons_console_game STATIC runtime/src/game.c)
	target_link_libraries(simpsons_console_game PUBLIC simpsons_console_program simpsons_video simpsons_sound
		simpsons_assets)
	# (its pack's version: the same layout and the same src/table.c as The Simpsons Arcade Game's, so the same pack)
	simpsons_add_assets_pack(simpsons_assets_pack_console simpsons_console_program)
	target_link_libraries(simpsons_console_game PUBLIC simpsons_assets_pack_console)
	# simpsons_console_headless: The Simpsons Mr. Plow Edition with no window, for quick checks (ports/headless/main.c:
	# its menus' actions, WIDE, its frames as pictures, and it ends at --frames; as tmnt-port's tmnt_console_headless),
	# its letters' font.bmp beside it
	if(NOT SIMPSONS_DEVICE)
		add_executable(simpsons_console_headless ports/headless/main.c)
		target_link_libraries(simpsons_console_headless PRIVATE simpsons_console_game)
		simpsons_game_stack(simpsons_console_headless)
		add_custom_command(TARGET simpsons_console_headless POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ports/sdl2/font/font.bmp
				$<TARGET_FILE_DIR:simpsons_console_headless>)
	endif()
	# MULTIPLAYER, the LAN game (console/PLAN.md, 11): arcade-lan, a repository of its own (lan/, or ARCADE_LAN_DIR
	# where it is), as tmnt-port's; without it the LAN page says NO NETWORK. iOS refuses broadcasts to an app without
	# Apple's multicast entitlement: the rooms there found by the library's SEEKs and sweeps (ARCADE_LAN_NO_BROADCAST)
	set(ARCADE_LAN_DIR ${CMAKE_SOURCE_DIR}/lan CACHE PATH "arcade-lan's sources, for The Simpsons' LAN game")
	if(EXISTS ${ARCADE_LAN_DIR}/CMakeLists.txt AND (NOT SIMPSONS_UWP OR SIMPSONS_UWP_LAN))
		if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
			set(ARCADE_LAN_NO_BROADCAST ON CACHE BOOL "" FORCE)
		endif()
		add_subdirectory(${ARCADE_LAN_DIR} ${CMAKE_BINARY_DIR}/arcade_lan)
		target_link_libraries(simpsons_console_program PUBLIC arcade_lan)
		target_compile_definitions(simpsons_console_program PUBLIC SIMPSONS_LAN)
	endif()
endif()

# SDL_GameControllerDB (ports/sdl2/controllerdb, zlib License): the community's mappings of game controllers, this
# platform's lines built into the window port's games (a player's report on tmnt-port: an Nvidia Shield's PS4 and Xbox
# controllers). SIMPSONS_CONTROLLER_DB to whatever links it: main.c gives SDL the mappings as it starts. The Switch and
# the Anbernic keep their own controls
if(SIMPSONS_WINDOW_PROGRAM AND NOT SIMPSONS_SWITCH AND NOT SIMPSONS_ANBERNIC)
	if(ANDROID)
		set(SIMPSONS_CONTROLLER_PLATFORM "Android")
	elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
		set(SIMPSONS_CONTROLLER_PLATFORM "iOS")
	elseif(APPLE)
		set(SIMPSONS_CONTROLLER_PLATFORM "Mac OS X")
	elseif(WIN32)
		set(SIMPSONS_CONTROLLER_PLATFORM "Windows")
	else()
		set(SIMPSONS_CONTROLLER_PLATFORM "Linux")
	endif()
	set(SIMPSONS_CONTROLLER_DB_C ${CMAKE_BINARY_DIR}/controller_db.c)
	add_custom_command(OUTPUT ${SIMPSONS_CONTROLLER_DB_C}
		COMMAND ${CMAKE_COMMAND} -DDB=${CMAKE_SOURCE_DIR}/ports/sdl2/controllerdb/gamecontrollerdb.txt
			"-DPLATFORM=${SIMPSONS_CONTROLLER_PLATFORM}" -DOUT=${SIMPSONS_CONTROLLER_DB_C}
			-P ${CMAKE_SOURCE_DIR}/ports/sdl2/controllerdb/embed.cmake
		DEPENDS ${CMAKE_SOURCE_DIR}/ports/sdl2/controllerdb/gamecontrollerdb.txt
			${CMAKE_SOURCE_DIR}/ports/sdl2/controllerdb/embed.cmake
		COMMENT "SDL_GameControllerDB's mappings for ${SIMPSONS_CONTROLLER_PLATFORM}"
		VERBATIM)
	add_library(simpsons_controller_db STATIC ${SIMPSONS_CONTROLLER_DB_C})
	set_target_properties(simpsons_controller_db PROPERTIES POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(simpsons_controller_db INTERFACE SIMPSONS_CONTROLLER_DB)
endif()

# CONTROLS' pictures (ports/sdl2/prompts: Kenney's Input Prompts Pixel, CC0) built into the games that have no file
# beside them: the Switch's, an .nro or an .nsp's executable alone. SIMPSONS_PROMPTS_EMBEDDED to whatever links it:
# main.c reads the sheet from there. And two checks on a computer, never a player's build: SIMPSONS_PAD_CHECK, the
# games with --pads (SDL's virtual game controllers, and --actions' "pad": CONTROLS binds a controller's buttons with
# none plugged in), and SIMPSONS_SWITCH_CHECK, The Simpsons Mr. Plow Edition as the Switch has it (its menus, WIDE
# 16:9, CONTROLS with the Switch's buttons' pictures out of the executable), with --pads
option(SIMPSONS_PAD_CHECK "Build the games with --pads, SDL's virtual game controllers, to check CONTROLS" OFF)
option(SIMPSONS_SWITCH_CHECK "Build The Simpsons Mr. Plow Edition as the Switch's, to check it on a computer" OFF)
if(SIMPSONS_WINDOW_PROGRAM AND (SIMPSONS_SWITCH OR SIMPSONS_SWITCH_CHECK))
	set(SIMPSONS_PROMPTS_C ${CMAKE_BINARY_DIR}/prompts_sheet.c)
	add_custom_command(OUTPUT ${SIMPSONS_PROMPTS_C}
		COMMAND ${CMAKE_COMMAND} -DSHEET=${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/input_prompts.bmp
			-DOUT=${SIMPSONS_PROMPTS_C} -P ${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/embed.cmake
		DEPENDS ${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/input_prompts.bmp
			${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/embed.cmake
		COMMENT "CONTROLS' pictures into the executable"
		VERBATIM)
	add_library(simpsons_prompts STATIC ${SIMPSONS_PROMPTS_C})
	set_target_properties(simpsons_prompts PROPERTIES POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(simpsons_prompts INTERFACE SIMPSONS_PROMPTS_EMBEDDED)
endif()

# The games in a window (ports/sdl2): The Simpsons Arcade Game, the decompiled program where src/ has it
# (SIMPSONS_WINDOW_PROGRAM), else the translation scaffold; and The Simpsons, the console version. SDL2 from the
# system, or on Windows the development package unpacked in build/downloads/sdl2, its library beside the executables;
# or, with SIMPSONS_STATIC_SDL2, SDL2 built from its own source into them. On a device, the device's own build
if(SIMPSONS_WINDOW_PROGRAM AND SIMPSONS_SWITCH)
	include(ports/switch/switch.cmake)
elseif(SIMPSONS_WINDOW_PROGRAM AND SIMPSONS_ANBERNIC)
	include(ports/anbernic/anbernic.cmake)
elseif(SIMPSONS_WINDOW_PROGRAM AND SIMPSONS_ANDROID)
	include(ports/android/android.cmake)
elseif(SIMPSONS_WINDOW_PROGRAM AND SIMPSONS_IOS)
	include(ports/ios/ios.cmake)
elseif(SIMPSONS_WINDOW_PROGRAM AND SIMPSONS_UWP)
	include(ports/uwp/uwp.cmake)
endif()
if(SIMPSONS_WINDOW_PROGRAM AND NOT SIMPSONS_DEVICE)
	if(SIMPSONS_STATIC_SDL2)
		include(FetchContent)
		if(EXISTS ${SIMPSONS_SDL2_SOURCE}/CMakeLists.txt)
			set(FETCHCONTENT_SOURCE_DIR_SDL2 ${SIMPSONS_SDL2_SOURCE})
		endif()
		FetchContent_Declare(SDL2 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-2.32.10
			GIT_SHALLOW TRUE)
		set(SDL_SHARED OFF CACHE BOOL "" FORCE)
		set(SDL_STATIC ON CACHE BOOL "" FORCE)
		set(SDL_TEST OFF CACHE BOOL "" FORCE)
		set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)    # (MSVC: the C runtime inside, as the game's)
		set(SDL_CCACHE OFF CACHE BOOL "" FORCE)
		FetchContent_MakeAvailable(SDL2)
		set(SIMPSONS_SDL2 SDL2::SDL2-static)
		set(SIMPSONS_SDL2_LICENSE ${sdl2_SOURCE_DIR}/LICENSE.txt)
		set(SDL2_FOUND TRUE)
	else()
		set(SIMPSONS_SDL2_UNPACKED "")
		if(WIN32)
			file(GLOB SIMPSONS_SDL2_UNPACKED ${CMAKE_SOURCE_DIR}/build/downloads/sdl2/SDL2-*/cmake)
		endif()
		find_package(SDL2 CONFIG QUIET HINTS ${SIMPSONS_SDL2_UNPACKED})
		set(SIMPSONS_SDL2 SDL2::SDL2)
		file(GLOB SIMPSONS_SDL2_LICENSE ${CMAKE_SOURCE_DIR}/build/downloads/sdl2/SDL2-*/LICENSE.txt)
	endif()
	if(SDL2_FOUND)
		if(TARGET SDL2::SDL2main)
			list(APPEND SIMPSONS_SDL2 SDL2::SDL2main)
		endif()

		# the executables' icon on Windows, which SDL gives the windows too: the project's (ports/sdl2/icon/simpsons.ico,
		# Homer at the window, the picture Sebas chose for the computers and the devices, made of simpsons.png by
		# make_ico.py), or a picture of the player's own where this computer has one (SIMPSONS_PC_ICON, else
		# local/icon_pc.png: local/ holds the player's own pictures, never committed), made an .ico here
		# (ports/sdl2/icon/make_ico.py, Pillow's)
		set(SIMPSONS_PC_ICON "" CACHE FILEPATH "The executables' icon on Windows: a picture of the player's own")
		set(SIMPSONS_PC_PICTURE ${SIMPSONS_PC_ICON})
		if(NOT SIMPSONS_PC_PICTURE AND EXISTS ${CMAKE_SOURCE_DIR}/local/icon_pc.png)
			set(SIMPSONS_PC_PICTURE ${CMAKE_SOURCE_DIR}/local/icon_pc.png)
		endif()
		set(SIMPSONS_ICON_RC "")
		if(WIN32 AND SIMPSONS_PC_PICTURE AND Python3_FOUND)
			# (made as CMake configures: again when the picture changes)
			set(SIMPSONS_OWN_ICON ${CMAKE_BINARY_DIR}/icon)
			execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/ports/sdl2/icon/make_ico.py
				${SIMPSONS_PC_PICTURE} ${SIMPSONS_OWN_ICON}/own_icon.ico RESULT_VARIABLE SIMPSONS_OWN_ICON_MADE)
			set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SIMPSONS_PC_PICTURE})
			if(SIMPSONS_OWN_ICON_MADE EQUAL 0)
				file(WRITE ${SIMPSONS_OWN_ICON}/own_icon.rc
					"// the player's own picture (CMakeLists.txt)\n1 ICON \"own_icon.ico\"\n")
				set(SIMPSONS_ICON_RC ${SIMPSONS_OWN_ICON}/own_icon.rc)
				message(STATUS "The executables' icon: ${SIMPSONS_PC_PICTURE}")
			else()
				message(WARNING "${SIMPSONS_PC_PICTURE} could not be made an icon (ports/sdl2/icon/make_ico.py, Pillow)")
			endif()
		endif()
		if(WIN32 AND NOT SIMPSONS_ICON_RC)
			# (the project's: no Pillow needed)
			set(SIMPSONS_OWN_ICON ${CMAKE_BINARY_DIR}/icon)
			file(TO_CMAKE_PATH ${CMAKE_SOURCE_DIR}/ports/sdl2/icon/simpsons.ico SIMPSONS_PROJECT_ICO)
			file(WRITE ${SIMPSONS_OWN_ICON}/project_icon.rc
				"// the project's icon (CMakeLists.txt)\n1 ICON \"${SIMPSONS_PROJECT_ICO}\"\n")
			set(SIMPSONS_ICON_RC ${SIMPSONS_OWN_ICON}/project_icon.rc)
			message(STATUS "The executables' icon: ports/sdl2/icon/simpsons.ico")
		endif()

		# A computer's check of Android's touch screen and its WIDE by the screen's shape (never a player's build): the
		# games with the touch controls drawn (ports/sdl2/touch.c, SIMPSONS_TOUCH) and main.c's --screen WIDTHxHEIGHT,
		# which writes each screen --video-dump names as it is presented (SIMPSONS_TOUCH_CHECK), with SDL's dummy video
		option(SIMPSONS_TOUCH_CHECK "Build the games with Android's touch controls, to check them on a computer" OFF)

		# a game's executable: the window port around 'program', with its first run (ports/sdl2/first_run.c: the assets
		# pack out of the player's ROM set, its window written with Kenney Mini, CC0, ports/sdl2/font); its icon; its
		# pictures (font.bmp, and the CONTROLS page's input_prompts.bmp) beside it, in the .app's Resources on macOS (a
		# bundle, 'identifier' its own: SDL_GetBasePath finds them there), and on Linux the icon's picture beside it
		# (ports/sdl2/icon/simpsons.png: the desktop entries take it); SDL2's library
		# beside it unless it is inside; the game's stack
		# (a ROM set packed as .7z: the LZMA SDK's 7z decoder, public domain, ports/sdl2/lzma, unchanged)
		if(NOT TARGET simpsons_7z)
			file(GLOB SIMPSONS_7Z_SOURCES ${CMAKE_SOURCE_DIR}/ports/sdl2/lzma/*.c)
			add_library(simpsons_7z STATIC ${SIMPSONS_7Z_SOURCES} ports/sdl2/rom_set_7z.c)
			target_include_directories(simpsons_7z PUBLIC ports/sdl2)
			target_compile_definitions(simpsons_7z PRIVATE Z7_EXTRACT_ONLY)
		endif()
		function(simpsons_window target name identifier program)
			add_executable(${target} WIN32 MACOSX_BUNDLE ports/sdl2/main.c ports/sdl2/first_run.c ports/sdl2/controls.c
				ports/sdl2/letters.c)
			target_include_directories(${target} PRIVATE ports/sdl2)
			target_link_libraries(${target} PRIVATE ${program} simpsons_controller_db simpsons_7z ${SIMPSONS_SDL2})
			set_target_properties(${target} PROPERTIES OUTPUT_NAME ${name}
				MACOSX_BUNDLE_BUNDLE_NAME ${name} MACOSX_BUNDLE_GUI_IDENTIFIER ${identifier}
				MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION} MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
			simpsons_game_stack(${target})
			if(SIMPSONS_TOUCH_CHECK)
				target_sources(${target} PRIVATE ports/sdl2/touch.c)
				target_compile_definitions(${target} PRIVATE SIMPSONS_TOUCH SIMPSONS_TOUCH_CHECK)
			endif()
			if(SIMPSONS_PAD_CHECK OR SIMPSONS_SWITCH_CHECK)
				target_compile_definitions(${target} PRIVATE SIMPSONS_PAD_CHECK)
			endif()
			if(SIMPSONS_ICON_RC)
				target_sources(${target} PRIVATE ${SIMPSONS_ICON_RC})
			endif()
			if(APPLE)
				target_sources(${target} PRIVATE ports/sdl2/font/font.bmp ports/sdl2/prompts/input_prompts.bmp)
				set_source_files_properties(ports/sdl2/font/font.bmp ports/sdl2/prompts/input_prompts.bmp
					PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
				if(EXISTS ${CMAKE_SOURCE_DIR}/ports/sdl2/icon/simpsons.icns)
					target_sources(${target} PRIVATE ports/sdl2/icon/simpsons.icns)
					set_source_files_properties(ports/sdl2/icon/simpsons.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
					set_target_properties(${target} PROPERTIES MACOSX_BUNDLE_ICON_FILE simpsons.icns)
				endif()
				# (first_run.c's app_folder: the folder a translocated .app was put in, CoreFoundation's URLs)
				target_link_libraries(${target} PRIVATE "-framework CoreFoundation")
			else()
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ports/sdl2/font/font.bmp
						${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/input_prompts.bmp $<TARGET_FILE_DIR:${target}>)
			endif()
			if(NOT WIN32 AND NOT APPLE)
				target_link_libraries(${target} PRIVATE m)    # (main.c's)
				if(EXISTS ${CMAKE_SOURCE_DIR}/ports/sdl2/icon/simpsons.png)
					add_custom_command(TARGET ${target} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ports/sdl2/icon/simpsons.png
							$<TARGET_FILE_DIR:${target}>)
				endif()
			endif()
			if(WIN32 AND NOT SIMPSONS_STATIC_SDL2)
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:${target}>)
			endif()
		endfunction()

		# The Simpsons Arcade Game, the arcade game as it is: its EEPROM kept (simpsons.nv), its players' controls in
		# controls.ini
		simpsons_window(simpsons "The Simpsons Arcade Game" io.github.cheviman.simpsons-arcade ${SIMPSONS_WINDOW_PROGRAM})
		# The Simpsons, the console version: its settings in simpsons.ini, OPTIONS > CONTROLS drawing the keys and
		# buttons with Kenney's Input Prompts Pixel (CC0, ports/sdl2/prompts)
		if(TARGET simpsons_console_game)
			simpsons_window(simpsons_console "The Simpsons Mr. Plow Edition" io.github.cheviman.simpsons simpsons_console_game)
			if(SIMPSONS_SWITCH_CHECK)
				# (ports/switch/switch.cmake's definitions, and the Switch's pictures of the buttons)
				target_compile_definitions(simpsons_console_program PUBLIC SIMPSONS_HANDHELD SIMPSONS_SCREEN_WIDE
					SIMPSONS_SYSTEM_EXIT SIMPSONS_DEVICE_CONTROLS)
				target_compile_definitions(simpsons_console PRIVATE SIMPSONS_SWITCH_PROMPTS)
				target_link_libraries(simpsons_console PRIVATE simpsons_prompts)
			endif()
		endif()

		# The devices' makers (tmnt-port's), which the player runs on a computer with the player's own ROM set: its
		# assets pack as the first run makes it (ports/switch/nsp/rom_pack.c: the decompiled game's ROM set reader and
		# pack writer, so the devices' version), never a byte of the set in them. Simpsons NSP (ports/switch/nsp): The
		# Simpsons' and The Simpsons Arcade Game's .nsps out of the Switch build's pieces (the folder switch beside it), the
		# pack and the player's console keys. Simpsons Anbernic (ports/anbernic/maker): the folder PORTS for an Anbernic
		# H700 handheld's card out of the games' Anbernic build (the folder anbernic beside it) and the pack. Console
		# programs (SDL2 without SDL2main) whose window opens when they are started with nothing, its letters
		# (font.bmp) beside them; the folder dialog COM's on Windows
		if(TARGET simpsons_game_decompiled)
			list(GET SIMPSONS_SDL2 0 SIMPSONS_SDL2_LIBRARY)
			function(simpsons_maker target name definition)
				add_executable(${target} ${ARGN} ports/switch/nsp/rom_pack.c ports/switch/nsp/folder_dialog.c
					ports/sdl2/letters.c)
				target_include_directories(${target} PRIVATE ports/switch/nsp ports/sdl2)
				target_compile_definitions(${target} PRIVATE ${definition})
				target_link_libraries(${target} PRIVATE simpsons_game_decompiled ${SIMPSONS_SDL2_LIBRARY})
				set_target_properties(${target} PROPERTIES OUTPUT_NAME ${name})
				if(WIN32)
					target_link_libraries(${target} PRIVATE ole32 shell32 uuid)
				endif()
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ports/sdl2/font/font.bmp
						$<TARGET_FILE_DIR:${target}>)
				if(WIN32 AND NOT SIMPSONS_STATIC_SDL2)
					add_custom_command(TARGET ${target} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:${target}>)
				endif()
			endfunction()
			simpsons_maker(simpsons_nsp "Simpsons NSP" SIMPSONS_NSP_WINDOW ports/switch/nsp/main.c
				ports/switch/nsp/make_nsp.c ports/switch/nsp/crypto.c ports/switch/nsp/formats.c ports/switch/nsp/window.c)
			simpsons_maker(simpsons_anbernic_maker "Simpsons Anbernic" SIMPSONS_ANBERNIC_WINDOW
				ports/anbernic/maker/main.c ports/anbernic/maker/make_ports.c ports/anbernic/maker/window.c)
			target_compile_definitions(simpsons_anbernic_maker PRIVATE
				[[FOLDER_TITLE="The folder for PORTS (the card's Roms)"]])
			# Simpsons APK (ports/android/apk, tmnt-port's): The Simpsons' and The Simpsons Arcade Game's APKs out of the
			# Android build (android/simpsons-base.apk and simpsons-arcade-base.apk beside it), the pack and the player's
			# icon and TV banner, signed with a key of the player's own made once (v2's signature: no Java nor Android SDK
			# on the player's computer; BCryptGenRandom draws the key on Windows)
			simpsons_maker(simpsons_apk_maker "Simpsons APK" SIMPSONS_APK_WINDOW ports/android/apk/main.c
				ports/android/apk/make_apk.c ports/android/apk/zip.c ports/android/apk/sign.c ports/android/apk/key.c
				ports/android/apk/ecdsa.c ports/switch/nsp/crypto.c ports/android/apk/window.c)
			target_compile_definitions(simpsons_apk_maker PRIVATE
				[[FOLDER_TITLE="The folder for The Simpsons Mr. Plow Edition.apk and The Simpsons Arcade Game.apk"]])
			if(WIN32)
				target_link_libraries(simpsons_apk_maker PRIVATE bcrypt)
			endif()
		endif()

		# The Windows package (simpsons_package): what a player downloads, package/simpsons-windows-x64.zip in the build
		# folder (SIMPSONS_PACKAGE_DIR), a folder simpsons with The Simpsons Arcade Game.exe and The Simpsons Mr. Plow Edition.exe (SDL2
		# inside them with SIMPSONS_STATIC_SDL2, else SDL2.dll beside them), the pictures they write with (font.bmp,
		# input_prompts.bmp), the package's README.txt and recomplauncher.json (docs/package: how RecompLauncher sets
		# The Simpsons Mr. Plow Edition up, starts it and shows its settings), the project's LICENSE.txt, and licenses/
		# with the terms of what is in it or what it follows: MAME's and ymfm's (the board's video and sound), SDL2's,
		# SDL_GameControllerDB's, Kenney's and the LZMA SDK's. The two games alone; nothing of the ROM set
		set(SIMPSONS_PACKAGE_DIR ${CMAKE_BINARY_DIR}/package CACHE PATH "Where simpsons_package writes the package")
		if(WIN32 AND TARGET simpsons_console)
			set(SIMPSONS_PACKAGE_NAME simpsons-windows-x64)
			set(SIMPSONS_PACKED ${SIMPSONS_PACKAGE_DIR}/simpsons)
			set(SIMPSONS_PACKED_SDL2 "")
			if(NOT SIMPSONS_STATIC_SDL2)
				set(SIMPSONS_PACKED_SDL2 COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SDL2::SDL2> ${SIMPSONS_PACKED})
			endif()
			set(SIMPSONS_PACKED_SDL2_LICENSE "")
			if(SIMPSONS_SDL2_LICENSE)
				list(GET SIMPSONS_SDL2_LICENSE 0 SIMPSONS_SDL2_LICENSE_FILE)
				set(SIMPSONS_PACKED_SDL2_LICENSE COMMAND ${CMAKE_COMMAND} -E copy ${SIMPSONS_SDL2_LICENSE_FILE}
					${SIMPSONS_PACKED}/licenses/SDL2.txt)
			endif()
			set(SIMPSONS_PACKED_LAN "")    # (arcade-lan's terms, where The Simpsons has its LAN game)
			if(TARGET arcade_lan)
				set(SIMPSONS_PACKED_LAN COMMAND ${CMAKE_COMMAND} -E copy ${ARCADE_LAN_DIR}/LICENSE
					${SIMPSONS_PACKED}/licenses/arcade-lan.txt)
			endif()
			file(MAKE_DIRECTORY ${SIMPSONS_PACKAGE_DIR})
			add_custom_target(simpsons_package
				COMMAND ${CMAKE_COMMAND} -E rm -rf ${SIMPSONS_PACKED} ${SIMPSONS_PACKAGE_DIR}/${SIMPSONS_PACKAGE_NAME}.zip
				COMMAND ${CMAKE_COMMAND} -E make_directory ${SIMPSONS_PACKED}/licenses
				COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:simpsons> $<TARGET_FILE:simpsons_console>
					${CMAKE_SOURCE_DIR}/ports/sdl2/font/font.bmp ${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/input_prompts.bmp
					${CMAKE_SOURCE_DIR}/docs/package/recomplauncher.json
					${SIMPSONS_PACKED}
				${SIMPSONS_PACKED_SDL2}
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/docs/package/README.txt ${SIMPSONS_PACKED}/README.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${SIMPSONS_PACKED}/LICENSE.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/docs/package/licenses/MAME.txt
					${SIMPSONS_PACKED}/licenses/MAME.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/docs/package/licenses/LZMA-SDK.txt
					${SIMPSONS_PACKED}/licenses/LZMA-SDK.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tools/core/konami/reference/ymfm/LICENSE
					${SIMPSONS_PACKED}/licenses/ymfm.txt
				${SIMPSONS_PACKED_SDL2_LICENSE}
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/ports/sdl2/controllerdb/LICENSE
					${SIMPSONS_PACKED}/licenses/SDL_GameControllerDB.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/ports/sdl2/font/License.txt
					${SIMPSONS_PACKED}/licenses/Kenney-Mini.txt
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/License.txt
					${SIMPSONS_PACKED}/licenses/Kenney-Input-Prompts.txt
				${SIMPSONS_PACKED_LAN}
				COMMAND ${CMAKE_COMMAND} -E tar cf ${SIMPSONS_PACKAGE_NAME}.zip --format=zip simpsons
				WORKING_DIRECTORY ${SIMPSONS_PACKAGE_DIR}
				DEPENDS simpsons simpsons_console
				COMMENT "Packing The Simpsons Arcade Game and The Simpsons Mr. Plow Edition: ${SIMPSONS_PACKAGE_NAME}.zip"
				VERBATIM)
		endif()
	else()
		message(WARNING "No SDL2: the games (The Simpsons Arcade Game and The Simpsons Mr. Plow Edition) are not built, only the development "
			"tools. -DSIMPSONS_STATIC_SDL2=ON builds SDL2 from its source into the games, as the packages do; or SDL2 "
			"from the system, on Windows its development package for Visual C++ unpacked in build/downloads/sdl2 "
			"(build/downloads/sdl2/SDL2-2.x.y/cmake)")
	endif()
endif()
