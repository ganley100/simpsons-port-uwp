# Name:        uwp.cmake
# Description: the UWP build (Visual Studio, CMAKE_SYSTEM_NAME WindowsStore): The Simpsons Mr. Plow Edition and The
#              Simpsons Arcade Game as UWP apps, for an Xbox One or Series X|S in Developer Mode (and Windows 10 and 11),
#              each a signed .msix with its certificate and Visual C++'s runtime package beside it
# Version:     0.1.0
# Created:     21-11-2026
# Modified:    21-11-2026
#
# Included by CMakeLists.txt when CMAKE_SYSTEM_NAME is WindowsStore. Both games are the Switch's builds on the TV
# (SIMPSONS_HANDHELD: the screen whole, its refresh the system's, no first run's window; SIMPSONS_DEVICE_CONTROLS:
# OPTIONS > CONTROLS binds each player's controller's buttons), with EXIT kept in The Simpsons' menu (an app on the
# Xbox may end itself), The Simpsons' WIDE AUTO by the TV's shape (main.c's follow_screen: 16:9). SDL2 from its source,
# static, on its WinRT backend: Direct3D 11, the controllers through Windows.Gaming.Input (up to eight; the cabinet's
# four), WASAPI's sound; SDL's WinRT main (src/main/winrt/SDL_winrt_main_NonXAML.cpp, C++/CX: /ZW) in each app, which
# starts the game's main on the app's thread (the program needs less than 32 KB of stack, ports/PLAN.md A1; the
# executable still asks for 64 MB, simpsons_game_stack). Each app reads the assets pack the package carries
# (SIMPSONS_BIN_DIR/simpsons.assets, where this build has the player's: the package is then the player's alone, never
# shared), else the one in its LocalState folder, made there of the player's simpsons.zip at its first start (main.c's
# device_assets), where what it keeps and its log are too: on the Xbox, Device Portal's File explorer reaches it
# (LocalAppData > the package > LocalState). The C library of an app has no environment: uwp_compat.h before the
# game's C. The packages are signed with a key of this computer's (SIMPSONS_UWP_KEY, by default local/uwp_key.pfx,
# which make_key.ps1 makes when there is none: local/ is never in the repository; the same key each time, so a new
# package installs over the last one and keeps LocalState), its .cer beside the packages for Device Portal.
# docs/package/README-xbox.txt is the player's how-to.
#
# Usage (Windows, Visual Studio 2022 with "Universal Windows Platform development" and its C++ (v143) UWP tools):
#   cmake -S . -B build/uwp -G "Visual Studio 17 2022" -A x64 -DCMAKE_SYSTEM_NAME=WindowsStore
#       -DCMAKE_SYSTEM_VERSION=10.0 -DSIMPSONS_REFERENCE_TOOLS=OFF [-DSIMPSONS_BIN_DIR=build/bin-uwp]
#   cmake --build build/uwp --config Release --target simpsons_uwp_games
# The packages: SIMPSONS_BIN_DIR/uwp/packages/<app>_<version>_x64_Test/ (the .msix, the .cer, Dependencies/x64/ with
# Microsoft.VCLibs.x64.14.00.appx), one folder for each game.

if(NOT CMAKE_GENERATOR MATCHES "^Visual Studio")
	message(FATAL_ERROR "The UWP build is Visual Studio's (-G \"Visual Studio 17 2022\" -A x64): MSBuild puts the "
		"packages together and signs them")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(WARNING "The Xbox runs x64 apps: -A x64")
endif()

# (no EXIT taken out: SIMPSONS_SYSTEM_EXIT is not the Xbox's, whose Guide leaves an app only to the home screen)
set(SIMPSONS_UWP_DEFINITIONS SIMPSONS_HANDHELD SIMPSONS_DEVICE_CONTROLS)
target_compile_definitions(simpsons_console_program PUBLIC ${SIMPSONS_UWP_DEFINITIONS})

# The game's C with what a UWP app's C library lacks (getenv): the programs and the window port
set(SIMPSONS_UWP_COMPAT "/FI${CMAKE_SOURCE_DIR}/ports/uwp/uwp_compat.h")
foreach(program IN ITEMS simpsons_game_program simpsons_console_program simpsons_runtime)
	if(TARGET ${program})
		target_compile_options(${program} PRIVATE $<$<COMPILE_LANGUAGE:C>:${SIMPSONS_UWP_COMPAT}>)
	endif()
endforeach()

# SDL2 from its source, static, on WinRT (SDL's CMake builds its C++/CX with /ZW itself); its C runtime the dynamic
# one, as a UWP app's must be (Microsoft.VCLibs, beside the package)
include(FetchContent)
if(EXISTS ${SIMPSONS_SDL2_SOURCE}/CMakeLists.txt AND NOT FETCHCONTENT_SOURCE_DIR_SDL2)
	set(FETCHCONTENT_SOURCE_DIR_SDL2 ${SIMPSONS_SDL2_SOURCE})
endif()
FetchContent_Declare(SDL2 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-2.32.10 GIT_SHALLOW TRUE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_FORCE_STATIC_VCRT OFF CACHE BOOL "" FORCE)
set(SDL_CCACHE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)
set(SIMPSONS_UWP_MAIN ${sdl2_SOURCE_DIR}/src/main/winrt/SDL_winrt_main_NonXAML.cpp)

# The key that signs the packages: the player's own (SIMPSONS_UWP_KEY), else local/uwp_key.pfx, made once
# (make_key.ps1); the manifest's publisher is its subject
set(SIMPSONS_UWP_KEY "" CACHE FILEPATH "The .pfx the UWP packages are signed with (by default local/uwp_key.pfx, made once)")
set(SIMPSONS_UWP_PUBLISHER "CN=SimpsonsPort" CACHE STRING "The UWP packages' publisher: the signing key's subject")
set(SIMPSONS_UWP_PFX ${SIMPSONS_UWP_KEY})
if(NOT SIMPSONS_UWP_PFX)
	set(SIMPSONS_UWP_PFX ${CMAKE_SOURCE_DIR}/local/uwp_key.pfx)
	if(NOT EXISTS ${SIMPSONS_UWP_PFX})
		execute_process(COMMAND powershell -NoProfile -ExecutionPolicy Bypass
			-File ${CMAKE_SOURCE_DIR}/ports/uwp/make_key.ps1 -Pfx ${SIMPSONS_UWP_PFX}
			-Cer ${CMAKE_SOURCE_DIR}/local/uwp_key.cer -Subject ${SIMPSONS_UWP_PUBLISHER}
			RESULT_VARIABLE SIMPSONS_UWP_KEY_MADE)
		if(NOT SIMPSONS_UWP_KEY_MADE EQUAL 0 OR NOT EXISTS ${SIMPSONS_UWP_PFX})
			message(FATAL_ERROR "The UWP packages' key could not be made (ports/uwp/make_key.ps1): give one, "
				"-DSIMPSONS_UWP_KEY=<a .pfx without a password> -DSIMPSONS_UWP_PUBLISHER=<its subject>")
		endif()
		message(STATUS "The UWP packages' key: ${SIMPSONS_UWP_PFX} (made now, kept for the next builds)")
	endif()
endif()

# what each package carries beside the executable: the tiles, the console's letters and CONTROLS' pictures, and the
# player's assets pack where this build has one
# The tiles: the project's (ports/uwp/Assets, the donut), or made of a picture of the player's own where this computer
# has one (SIMPSONS_UWP_ICON, else local/icon_uwp.png: local/ holds the player's own pictures, the show's art, never
# committed), filling each tile, cut from the top down (make_logos.py, Pillow's), as CMake configures
set(SIMPSONS_UWP_ICON "" CACHE FILEPATH "The UWP apps' tiles: a picture of the player's own")
set(SIMPSONS_UWP_PICTURE ${SIMPSONS_UWP_ICON})
if(NOT SIMPSONS_UWP_PICTURE AND EXISTS ${CMAKE_SOURCE_DIR}/local/icon_uwp.png)
	set(SIMPSONS_UWP_PICTURE ${CMAKE_SOURCE_DIR}/local/icon_uwp.png)
endif()
set(SIMPSONS_UWP_LOGO_DIR ${CMAKE_SOURCE_DIR}/ports/uwp/Assets)
if(SIMPSONS_UWP_PICTURE AND Python3_FOUND)
	execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/ports/uwp/make_logos.py
		--picture ${SIMPSONS_UWP_PICTURE} ${CMAKE_BINARY_DIR}/uwp/tiles
		RESULT_VARIABLE SIMPSONS_UWP_TILES_MADE OUTPUT_QUIET)
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SIMPSONS_UWP_PICTURE})
	if(SIMPSONS_UWP_TILES_MADE EQUAL 0)
		set(SIMPSONS_UWP_LOGO_DIR ${CMAKE_BINARY_DIR}/uwp/tiles)
		message(STATUS "The UWP apps' tiles: ${SIMPSONS_UWP_PICTURE}")
	else()
		message(WARNING "${SIMPSONS_UWP_PICTURE} could not be made the tiles (ports/uwp/make_logos.py, Pillow): "
			"the project's are")
	endif()
elseif(SIMPSONS_UWP_PICTURE)
	message(WARNING "The UWP apps' tiles of ${SIMPSONS_UWP_PICTURE} need Python 3 with Pillow: the project's are")
endif()
set(SIMPSONS_UWP_LOGOS "")
foreach(logo IN ITEMS Square44x44Logo Square150x150Logo StoreLogo Wide310x150Logo SplashScreen)
	list(APPEND SIMPSONS_UWP_LOGOS ${SIMPSONS_UWP_LOGO_DIR}/${logo}.png)
endforeach()
set_source_files_properties(${SIMPSONS_UWP_LOGOS} PROPERTIES VS_DEPLOYMENT_CONTENT 1 VS_DEPLOYMENT_LOCATION "Assets")
set(SIMPSONS_UWP_CONTENT ${CMAKE_SOURCE_DIR}/ports/sdl2/font/font.bmp ${CMAKE_SOURCE_DIR}/ports/sdl2/prompts/input_prompts.bmp)
if(EXISTS ${SIMPSONS_BIN_DIR}/simpsons.assets)
	list(APPEND SIMPSONS_UWP_CONTENT ${SIMPSONS_BIN_DIR}/simpsons.assets)
	message(STATUS "The UWP packages carry ${SIMPSONS_BIN_DIR}/simpsons.assets: they are yours alone")
endif()
set_source_files_properties(${SIMPSONS_UWP_CONTENT} PROPERTIES VS_DEPLOYMENT_CONTENT 1)

# (the LAN game's networking, where The Simpsons has it)
set(SIMPSONS_UWP_CAPABILITIES "")
if(TARGET arcade_lan)
	set(SIMPSONS_UWP_CAPABILITIES "
    <Capability Name=\"internetClientServer\" />
    <Capability Name=\"privateNetworkClientServer\" />")
endif()
set(SIMPSONS_UWP_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.0)
set(SIMPSONS_UWP_OUT ${SIMPSONS_BIN_DIR}/uwp)

# A game of the UWP: its app ('target', its executable 'executable', its name 'name', its package's identity
# 'identity' and the GUID 'guid'), the window port's main.c, controls.c and letters.c on 'program' with SDL's WinRT
# main; its manifest, its key, its tiles and files; its signed package (.msix) in SIMPSONS_UWP_OUT/packages
function(simpsons_uwp_game target executable name identity guid program)
	set(SIMPSONS_UWP_NAME ${name})
	set(SIMPSONS_UWP_IDENTITY ${identity})
	set(SIMPSONS_UWP_GUID ${guid})
	set(manifest ${CMAKE_BINARY_DIR}/uwp/${target}/Package.appxmanifest)
	configure_file(${CMAKE_SOURCE_DIR}/ports/uwp/Package.appxmanifest.in ${manifest} @ONLY)

	add_executable(${target} WIN32 ports/sdl2/main.c ports/sdl2/controls.c ports/sdl2/letters.c ${SIMPSONS_UWP_MAIN}
		${manifest} ${SIMPSONS_UWP_PFX} ${SIMPSONS_UWP_LOGOS} ${SIMPSONS_UWP_CONTENT})
	set_source_files_properties(${SIMPSONS_UWP_MAIN} PROPERTIES COMPILE_OPTIONS "/ZW;/EHsc")
	target_include_directories(${target} PRIVATE ports/sdl2)
	target_link_libraries(${target} PRIVATE ${program} simpsons_controller_db SDL2::SDL2-static)
	target_compile_definitions(${target} PRIVATE ${SIMPSONS_UWP_DEFINITIONS})
	target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${SIMPSONS_UWP_COMPAT}>)
	simpsons_game_stack(${target})
	foreach(config IN ITEMS "" _DEBUG _RELEASE _RELWITHDEBINFO _MINSIZEREL)
		# (each app its own folder: MSBuild lays the package out beside the executable)
		set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY${config} ${SIMPSONS_UWP_OUT}/${target})
	endforeach()
	# the package on every build, for sideloading (no bundle, no Store upload), signed with the key above
	file(TO_NATIVE_PATH "${SIMPSONS_UWP_OUT}/packages/" packages)
	set_target_properties(${target} PROPERTIES OUTPUT_NAME ${executable}
		VS_GLOBAL_GenerateAppxPackageOnBuild true
		VS_GLOBAL_AppxBundle Never
		VS_GLOBAL_UapAppxPackageBuildMode SideloadOnly
		VS_GLOBAL_AppxPackageSigningEnabled true
		VS_GLOBAL_AppxPackageDir ${packages}
		VS_GLOBAL_AppxSymbolPackageEnabled false)
endfunction()

simpsons_uwp_game(simpsons_uwp SimpsonsMrPlow "The Simpsons Mr. Plow Edition" SimpsonsPort.MrPlowEdition
	"6c1f8a52-3b0e-4d7a-9a51-0f3e5b6d2c11" simpsons_console_game)
simpsons_uwp_game(simpsons_arcade_uwp SimpsonsArcade "The Simpsons Arcade Game" SimpsonsPort.ArcadeGame
	"6c1f8a52-3b0e-4d7a-9a51-0f3e5b6d2c12" ${SIMPSONS_WINDOW_PROGRAM})
add_custom_target(simpsons_uwp_games)
add_dependencies(simpsons_uwp_games simpsons_uwp simpsons_arcade_uwp)
