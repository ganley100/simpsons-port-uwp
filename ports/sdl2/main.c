/**
 *    Name:        main.c
 *    Description: the game in a window: SDL2 shows its screen, plays its sound, reads the controls and keeps the
 *                 pace, around the same game and board the headless runs check
 *    Version:     0.2.0
 *    Created:     06-11-2026
 *    Modified:    15-11-2026
 *    Author:      Sebastian Toyos
 *
 *    license:BSD-3-Clause
 *    Usage: "The Simpsons Arcade Game" [--game ROMSET] [--prepare] [--script FILE [--fast]] [--hashes FILE]
 *                                 [--video-hashes FILE] [--sound-hashes FILE] [--service] [--assets FOLDER|PACK]
 *                                 [--video-dump FRAME ... --video-dir DIR] [--eeprom FILE] [--slow MS]
 *                                 [--no-vsync]
 *           "The Simpsons Mr. Plow Edition" [--game ROMSET] [--prepare] [--assets FOLDER|PACK] [--script FILE] [--fast]
 *                          [--actions FILE]
 *                          [--video-dump FRAME ... --video-dir DIR] [--wide KIND] [--wide-check] [--arcade-start]
 *                          [--stereo] [--scores FILE] [--state FILE [--state-save FRAME] [--state-load]]
 *                          [--slow MS] [--no-vsync]
 *
 *    The Simpsons Arcade Game, the arcade game as it is (runtime/src/game.c and the program): this file only wraps it.
 *    When each frame is done (game_frame_done) the port reads the keyboard and the game controllers into the board's
 *    inputs for the next one, closes the frame's sound and queues it, puts the frame in the window, and waits for the
 *    frame's time (the board's 59.185 a second) to pass. Its EEPROM (the test mode's settings) is kept in simpsons.nv,
 *    in the folder of what the game keeps, written when the game writes it; the factory one (the assets') where there
 *    is none.
 *
 *    The Simpsons, the console version (SIMPSONS_CONSOLE, console/): the same window around the console's game, which
 *    takes the players' controls and its menus' actions (console/console.h) and draws its own picture over each
 *    frame; its settings (OPTIONS: the cabinet's, the pixels, the volume) in simpsons.ini, in the folder of what the
 *    game keeps; the pause on Escape, a game controller's Back, or START of a player in the game. The high score table
 *    is kept in simpsons.scores (console/scores.c), and the pause's SAVE STATE writes the whole game's state in
 *    simpsons.state, which its LOAD STATE puts back (game/state.h: one state, the game's whole board, program, video,
 *    sound board and the console's, refused if of another version), both in the folder of what the game keeps.
 *
 *    The folder of what the game keeps (saves_folder): beside the executable on Windows, the user's own folder for
 *    the game elsewhere (SDL_GetPrefPath), or beside the game with a portable.txt there. There: simpsons.ini,
 *    simpsons.scores, simpsons.state, controls.ini (each player's keys and buttons, both games), simpsons.nv,
 *    gamecontrollerdb.txt (the player's own mappings, or beside the executable), and the log (simpsons.log,
 *    simpsons-arcade.log, the two runs before as .1.log and .2.log) when the game was started with nowhere for it to go
 *    (not from a terminal).
 *
 *    The game's data is its assets (game/assets.h), out of the player's ROM set: --assets (a folder or a pack); else
 *    the pack simpsons.assets beside the executable (beside the .app on macOS) or in the folder of what the game keeps;
 *    else the folder 'assets' tools/extract_assets.py writes, beside the executable or up to three folders above it,
 *    or in the current folder (for development); else the first run's (first_run.c): the pack made from the ROM set,
 *    simpsons.zip or the folder rom beside the game or in the folder of what the game keeps, or one dropped on its
 *    window, written in the folder of what the game keeps. A pack the game cannot load (another version's: the
 *    version is in its header) is made again from the ROM set; with none there, the window says so and asks for it.
 *    Both games read the same pack (the same layout and the same program's tables: the same version). On the
 *    computers --game ROMSET (or --game=ROMSET, one argument) names the player's ROM set, its .zip or its folder, taken
 *    before the others when the pack has to be made, as one dropped on the first run's window (a shortcut's, a
 *    launcher's); and --prepare makes the pack so with no window and ends: a launcher's setup step (RecompLauncher's,
 *    docs/package/recomplauncher.json), its progress on the standard output as lines "progress FRACTION TEXT", 0 when
 *    the pack is ready and 2 when it cannot be made. Without them the game starts as it always has.
 *
 *    On a device (SIMPSONS_HANDHELD, as tmnt-port's: the Nintendo Switch, ports/switch; Anbernic's H700 handhelds,
 *    ports/anbernic) the screen is the window, whole (the Switch's 1280 x 720, The Simpsons' WIDE 16:9 there,
 *    SIMPSONS_SCREEN_WIDE; the handheld's 640 x 480), its refresh the system's (no VSYNC), and there is no first run's
 *    window: the pack a maker put in (the Switch's romfs, beside the handheld's executables), else the device's
 *    folder's (device_folder: sdmc:/switch/simpsons-arcade/, PORTS/simpsons/), made there from the player's simpsons.zip at
 *    the first start; with none, a message that says where to put it. What the games keep is in that folder too. The
 *    handheld's buttons are a game controller's, its MENU leaves the game and its volume keys are The Simpsons' VOLUME
 *    (SIMPSONS_HANDHELD_KEYS); the Switch's HOME leaves it (SIMPSONS_SYSTEM_EXIT, the console's menus).
 *
 *    On Android (ports/android, as tmnt-port's) the screen is the phone's, whole (SDL's immersive full screen), The
 *    Simpsons' WIDE AUTO chosen by its shape and again whenever its size changes (follow_screen: a Fold opened mid-game);
 *    the touch screen's controls are player 1's (SIMPSONS_TOUCH, touch.c), hidden by a key or a game controller till a
 *    finger comes back; a game controller goes before a TV's remote, whose back is JUMP in The Simpsons. The pack is
 *    the APK's own (Simpsons APK puts the player's in), else the app's folder's on the storage (device_folder:
 *    Android/data/<the app>/files/, where the log is too), made there of the player's simpsons.zip; what the game
 *    keeps is in the app's own folder. Back is the menus' back and the pause, and the app going to the background
 *    pauses a game.
 *
 *    On iOS and iPadOS (ports/ios) as on Android: the screen whole (the status bar hidden, the home indicator dimmed),
 *    the picture between the sides the notch or the Dynamic Island takes (ios_side_inset), the touch screen's controls
 *    and the game controllers iOS knows; the pack and the ROM set in the app's Documents folder, which the Files app
 *    shows as the app's own (device_folder), where what the game keeps and its log are too; the app going to the
 *    background pauses a game and draws nothing there. The home gesture leaves it (SIMPSONS_SYSTEM_EXIT).
 *
 *    On the Xbox (ports/uwp: a UWP app, for the console's Developer Mode, and Windows 10 and 11 alike; __WINRT__) as
 *    on the Switch: the TV's screen whole, The Simpsons' WIDE AUTO by its shape, each controller a player and OPTIONS >
 *    CONTROLS binding their buttons (SIMPSONS_DEVICE_CONTROLS). The pack is the package's own (uwp.cmake puts
 *    SIMPSONS_BIN_DIR/simpsons.assets in, where this build has it), else the app's LocalState folder's (device_folder:
 *    Device Portal's File explorer, LocalAppData > the package > LocalState), made there of the player's simpsons.zip;
 *    what the game keeps and its log are there too. The controller's B is a button, never the system's back.
 *
 *    --script plays a control script (runtime/include/simpsons/script.h) instead of the keyboard, to its end;
 *    --fast runs without waiting for the frames' time. --slow MS makes each frame drawn take MS milliseconds more,
 *    as a slow device's, and says on the standard error every 600 frames how the pace held: the frames a second (the
 *    board's 59.19), how many were drawn and shown, and how often the sound ran out; --no-vsync runs it without
 *    VSYNC, as the devices' builds do (their screens keep the system's refresh), so that the two together are a slow
 *    device's frames on a computer. In a check's build (SIMPSONS_PAD_CHECK, CMake's option), --pads N plugs in N
 *    game controllers of SDL's own making and --actions' "FRAME pad BUTTON PLAYER" presses their buttons: CONTROLS'
 *    bindings with no controller there. --hashes, --video-hashes and --sound-hashes write for each
 *    frame the hashes kref writes, so a run in the window compares with the reference frame by frame. --service holds
 *    the test switch all along (from the start: the board then resets its EEPROM). --video-dump writes the frames
 *    FRAME ... as the window shows them (the console's picture over them) into DIR, frame_FRAME.bmp. In the console
 *    version --actions FILE plays its menus' actions, a line each: the frame, then up, down, left, right, accept,
 *    back, start, pause or dump, and a player (1-4) for his own stick, buttons or START (start: player 1's when none);
 *    the actions after a pause in its frame are the pause's, one a picture it shows (dump writes it, pause_FRAME_N.bmp).
 *    --wide KIND (off, 16:10, 16:9, 20:9, auto, or its number) plays it with OPTIONS > VIDEO > WIDE so (a scripted
 *    run's settings are the defaults: WIDE AUTO, the screen's shape's, OFF with SDL's dummy video and its 1024 x 768
 *    desktop, and OFF with --arcade-start), the pictures written as wide as WIDE draws them; --wide-check says, each
 *    frame, which columns of WIDE's sides are not their layer's map's; --arcade-start runs the arcade's own attract
 *    mode and controls (a script's coins and buttons straight onto the board, player N's on slot N), so the
 *    arcade's scenarios (tests/controls) run in the console's build, its WIDE in their games; --stereo plays it with
 *    OPTIONS > SOUND STEREO (the YM2151's two outputs as the program pans them). --scores FILE keeps the high score
 *    table there in place of simpsons.scores; --state FILE is the pause's saved state in place of simpsons.state, and
 *    with it --state-save FRAME saves the game's state at the end of frame FRAME, --state-load loads it at the end of
 *    the first frame it can (the boot over), so that a run from a state compares with the one it was saved in, frame by
 *    frame (the state keeps a script's place too).
 *    Each picture written says on the standard output where the console stands. A scripted run (--script, --actions,
 *    the hashes, --video-dump) reads and writes no settings, controls, EEPROM, high scores or state: it starts from the
 *    factory's, but with --eeprom FILE (The Simpsons Arcade Game), which keeps the EEPROM there in place of simpsons.nv,
 *    --scores FILE and --state FILE (The Simpsons), for checks.
 *
 *    Controls (controls.ini; the cabinet has four players, each with a coin slot and a start button, which the
 *    factory settings leave unused: a coin and the attack button bring a player in; player 1 is Marge, 2 Homer, 3
 *    Bart, 4 Lisa), as MAME's:
 *      player 1: arrows, Z or left Ctrl attack, X or left Alt jump, Enter or 1 start
 *      player 2: R F D G, A attack, S jump, 2 start; player 3: I K J L, right Ctrl, right Shift, 3;
 *      player 4: the keypad's 8 2 4 6, 0 attack, its point jump, 4
 *      5, 6, 7, 8                     coins 1-4 (The Simpsons Arcade Game)
 *      F2                             the test switch (held: the test mode's menu; The Simpsons Arcade Game)
 *      Escape                         quit (The Simpsons: back, the pause)
 *      F11 or Alt+Enter               full screen
 *      F3                             the next kind of pixels: sharp, smooth, bilinear, CRT
 *    Game controllers, players 1 to 4 in the order they are plugged (a TV's remote after them): the pad or the left
 *    stick moves, A attacks, B jumps, Start the start button, Back the coin (The Simpsons: the pause).
 */
#include "controls.h"
#ifndef SIMPSONS_HANDHELD    /* (a device has no first run's window: device_assets) */
#include "first_run.h"
#endif
#include "game/assets.h"
#include "game/game.h"
#include "simpsons/machine.h"
#include "simpsons/program.h"
#include "simpsons/script.h"
#include "simpsons/video.h"
#ifdef SIMPSONS_CONSOLE
#include "console.h"
#include "game/state.h"    /* (the pause's SAVE STATE and LOAD STATE) */
#endif
#ifdef SIMPSONS_TOUCH
#include "touch.h"         /* (Android's touch screen: player 1's controls) */
#endif
#ifdef __ANDROID__
#include "game/data.h"     /* (the APK's pack, read whole: load_apk_pack) */
#endif

#include <SDL.h>

#ifdef __SWITCH__
#include <switch.h>        /* romfsInit: the game's own files, its .nsp's or .nro's; libnx's console (fail) */
#include <sys/stat.h>
#endif
#ifdef __ANDROID__
#include <sys/stat.h>      /* (mkdir: the console's letters out of the APK, android_letters) */
#endif
#if !defined(_WIN32) && !defined(SIMPSONS_HANDHELD)
#include <unistd.h>        /* (isatty: a computer's log, open_log) */
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
#include <SDL_syswm.h>     /* (the UIWindow, for its safe area: ios_side_inset) */
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#ifdef __WINRT__
#include <SDL_system.h>    /* (SDL_WinRTGetFSPathUTF8: the package's LocalState, uwp_local_folder) */
/* (the Xbox controller's B comes as SDLK_AC_BACK too, the system's back: not the menus' back nor the pause there) */
#define CASE_AC_BACK
#define AC_BACK_IS_BACK     0
#else
#define CASE_AC_BACK        case SDLK_AC_BACK:
#define AC_BACK_IS_BACK     1
#endif

#include <inttypes.h>
#include <math.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SIMPSONS_CONSOLE
#define TITLE               "The Simpsons Mr. Plow Edition"
#define FILE_NAME           "simpsons"
#define SETTINGS_FILE       "simpsons.ini"
#define SCORES_FILE         "simpsons.scores"    /* the high score table (console/scores.c) */
#define STATE_FILE          "simpsons.state"     /* the pause's saved state, one (game/state.h) */
#else
#define TITLE               "The Simpsons Arcade Game"
#define FILE_NAME           "simpsons-arcade"
#define EEPROM_FILE         "simpsons.nv"     /* the ER5911's 128 bytes, as MAME keeps them */
#endif
#define SAVES_NAME          "The Simpsons"    /* the folder of what both games keep, where it is the user's own */
#ifdef __SWITCH__
#define SWITCH_FOLDER       "sdmc:/switch/simpsons-arcade"    /* both games' folder on the SD card: the ROM set, the
                                                                 pack, what they keep, their logs */
#define SWITCH_OLD_FOLDER   "sdmc:/switch/simpsons"    /* 1.0.0's, which The Simpsons: Hit & Run's port uses too (the
                                                          players' issue 2): switch_take_old_folder */
#endif
#define CONTROLS_FILE       "controls.ini"    /* (each player's keys and buttons: controls.c) */
#define PORTABLE_FILE       "portable.txt"    /* beside the game on Linux, macOS: what it keeps there (saves_folder) */
#define LOG_RUNS            3                 /* the runs whose logs are kept: .log, .1.log, .2.log (log_into) */
#define QUIT_HOLD_FRAMES    118               /* Back and Start held two seconds: The Simpsons Arcade Game ends */
#define MAPPINGS_FILE       "gamecontrollerdb.txt"    /* more game controllers for SDL (SDL_GameControllerDB's) */
#define PLAYERS             4
#define WINDOW_SCALE        3
#define STICK_DEAD_ZONE     8000
#define QUEUE_FRAMES_MAX    6          /* sound queued beyond this many frames is dropped */
#define MAX_DUMPS           64
/* the board's refresh: 6 MHz pixels, 384 a line, 264 lines */
#define FRAME_RATE_NUMERATOR    6000000u
#define FRAME_RATE_DENOMINATOR  (384u * 264u)

#ifdef SIMPSONS_HANDHELD
#define ROM_SET_USAGE       ""
#else
#define ROM_SET_USAGE       " [--game ROMSET] [--prepare]"    /* (the computers': find_assets, prepare) */
#endif
#ifdef SIMPSONS_CONSOLE
#define USAGE "Usage: \"" TITLE "\"" ROM_SET_USAGE " [--assets FOLDER|PACK] [--script FILE] [--fast] [--actions FILE]" \
	" [--video-dump FRAME ... --video-dir DIR] [--wide KIND] [--wide-check] [--arcade-start] [--stereo]" \
	" [--scores FILE] [--state FILE [--state-save FRAME] [--state-load]] [--lan-name NAME] [--lan-hashes FILE]" \
	" [--slow MS] [--no-vsync]"
#else
#define USAGE "Usage: \"" TITLE "\"" ROM_SET_USAGE " [--script FILE [--fast]] [--hashes FILE] [--video-hashes FILE]" \
	" [--sound-hashes FILE] [--service] [--assets FOLDER|PACK] [--video-dump FRAME ... --video-dir DIR] [--eeprom FILE]" \
	" [--slow MS] [--no-vsync]"
#endif

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_AudioDeviceID audio;
static SDL_GameController *controllers[PLAYERS];
#ifndef SIMPSONS_CONSOLE
static int arcade_pixels;             /* F3: the kind of pixels shown (the console's are its settings') */
#endif
static uint64_t next_frame;           /* performance counter value the next frame waits for */
static bool vsync_on;                 /* the renderer shows each frame at the screen's refresh (apply_vsync) */
static bool late_skip;                /* the next frame is not drawn: the last one ended behind its time
                                         (wait_for_frame) */
static int late_skips;                /* the frames in a row not drawn so */
static int late_pace;                 /* the frames left of a slow machine's pace, each frame drawn shown */
static int late_since = 1000;         /* the frames since one was last not drawn so */
static bool left_out;                 /* the last picture was not shown, behind its time (present) */
static bool no_vsync;                 /* --no-vsync: VSYNC off whatever the settings say (a check's: a device's) */
static int slow_ms;                   /* --slow MS: each frame drawn takes MS milliseconds more (a check's: a slow
                                         device's), and slow_frame says how the pace held */
static struct
{
	uint64_t since;                   /* (the performance counter as the count began) */
	unsigned frames, drawn, shown, sound_empty;
} slow;
static simpsons_script *script;
static uint64_t script_frames;
static bool fast;
#ifndef SIMPSONS_HANDHELD
static const char *rom_set_option;    /* --game: the player's ROM set, for the pack (find_assets, prepare) */
#endif
#ifdef SIMPSONS_CONSOLE
static bool launcher_full_screen;     /* full screen for RecompLauncher's Big Picture alone: simpsons.ini keeps its
                                         window (launcher_big_picture, save_settings) */
#endif
static bool unseen;                   /* --fast with SDL's dummy video: no picture shown (a check's run: its pictures
                                         are written, --video-dump) */
static bool service_held;             /* --service: the test switch on all along */
static bool scripted;                 /* a checked run: no files of the player's read or written */
static FILE *hashes;
static uint64_t frames_before;        /* the run's frames before a LAN game's board reset: --actions and --video-dump
                                         count on across it */
#ifdef SIMPSONS_CONSOLE
static FILE *lan_hashes;              /* --lan-hashes: each LAN frame's state hash, two devices' runs compared */
static bool lan_given;                /* a LAN frame's controls on the board: its hash after the frame */
static jmp_buf lan_restart_point;     /* (main's: a LAN game's start leaves the program there, the board reset) */
#endif
static FILE *video_hashes;
static FILE *sound_hashes;
static uint64_t dump_frames[MAX_DUMPS];
static int dump_count;
static const char *dump_dir = ".";

static void write_picture(const char *name);

/* ---- the picture the window shows: the frame the video drew, the console's own over it ---- */

#ifdef SIMPSONS_CONSOLE
/* (the board's 320 columns, or WIDE's wider frame: its rows SIMPSONS_VIDEO_MAX_WIDTH apart) */
static uint32_t picture_pixels[SIMPSONS_VIDEO_MAX_WIDTH * SIMPSONS_VIDEO_HEIGHT];
static console_picture picture = { picture_pixels, SIMPSONS_VIDEO_MAX_WIDTH, 0, 0 };
static bool wide_check;               /* --wide-check: each frame, WIDE's sides' columns that are not their map's */
#else
static uint32_t picture_pixels[SIMPSONS_VIDEO_WIDTH * SIMPSONS_VIDEO_HEIGHT];
#endif

/* ---- the pixels: sharp, smooth ("sharp bilinear": a whole number of times with sharp pixels into 'scaled',
   then that softened the rest of the way, so only the pixels' edges blend), plain bilinear, or CRT (a
   multiplying overlay on smooth pixels, the size the picture is shown at), as tmnt-port's ---- */

#define PIXEL_KINDS        4
#define CRT_SCANLINE_DARK  0.45    /* how much the scanlines' edges darken (their middles keep it all) */
#define CRT_MASK           0.80    /* the other two channels in a phosphor column */
#define CRT_VIGNETTE       0.18    /* how much the corners darken */

static SDL_Texture *scaled;
static int scaled_times, scaled_width;
static SDL_Texture *crt_overlay;
static int crt_width, crt_height;

/* For each pixel of the shown picture: its game row's scanline (bright in the middle, dark at the edges),
   the column's phosphor (red, green, blue in turn), and the vignette toward the edges */
static void make_crt_overlay(int width, int height)
{
	uint32_t *pixels;
	int pitch = 0;

	if (crt_overlay && crt_width == width && crt_height == height)
		return;
	if (crt_overlay)
		SDL_DestroyTexture(crt_overlay);
	crt_overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	crt_width = width;
	crt_height = height;
	if (!crt_overlay || SDL_LockTexture(crt_overlay, NULL, (void **)&pixels, &pitch) != 0)
		return;
	for (int y = 0; y < height; y++)
	{
		const double row = (y + 0.5) * SIMPSONS_VIDEO_HEIGHT / height;
		const double scan = 1.0 - CRT_SCANLINE_DARK * (1.0 - sin(3.14159265358979 * (row - floor(row))));
		const double dy = (2.0 * y / height - 1.0);

		for (int x = 0; x < width; x++)
		{
			const double dx = (2.0 * x / width - 1.0);
			const double light = scan * (1.0 - CRT_VIGNETTE * (dx * dx * dy * dy));
			const int phosphor = x % 3;
			const int red = (int)(255.0 * light * (phosphor == 0 ? 1.0 : CRT_MASK));
			const int green = (int)(255.0 * light * (phosphor == 1 ? 1.0 : CRT_MASK));
			const int blue = (int)(255.0 * light * (phosphor == 2 ? 1.0 : CRT_MASK));

			pixels[y * (pitch / 4) + x] = 0xFF000000u | (uint32_t)red << 16 | (uint32_t)green << 8 | (uint32_t)blue;
		}
	}
	SDL_UnlockTexture(crt_overlay);
	SDL_SetTextureBlendMode(crt_overlay, SDL_BLENDMODE_MOD);
}

/* The kind of pixels shown (0 sharp, 1 smooth, 2 bilinear, 3 CRT): the console's OPTIONS > VIDEO > PIXELS, the
   arcade's F3 */
static int pixels_kind(void)
{
#ifdef SIMPSONS_CONSOLE
	return console_settings.pixels;
#else
	return arcade_pixels;
#endif
}

static bool integer_scale(void)
{
#ifdef SIMPSONS_CONSOLE
	return console_settings.integer_scale;
#else
	return false;
#endif
}

#ifdef SIMPSONS_CONSOLE
/* ---- OPTIONS > VIDEO > FPS: the frames shown a second, at the picture's top right and in the window's title, as
   tmnt-port's ---- */

#define FPS_GLYPHS "0123456789 FPS"

/* FPS_GLYPHS' letters, 3 x 5 pixels of the game's: a row each, its high bit the left */
static const uint8_t fps_glyphs[][5] = {
	{ 7, 5, 5, 5, 7 }, { 2, 6, 2, 2, 7 }, { 7, 1, 7, 4, 7 }, { 7, 1, 7, 1, 7 }, { 5, 5, 7, 1, 1 },
	{ 7, 4, 7, 1, 7 }, { 7, 4, 7, 5, 7 }, { 7, 1, 2, 2, 2 }, { 7, 5, 7, 5, 7 }, { 7, 5, 7, 1, 7 },
	{ 0, 0, 0, 0, 0 }, { 7, 4, 6, 4, 4 }, { 6, 5, 6, 4, 4 }, { 3, 4, 2, 1, 6 }
};
static int fps_counted = -1;    /* the last half second's frames a second (-1: none counted yet) */

/* A frame shown: the frames a second, each half second; the window says them in its title */
static void count_fps(void)
{
	static uint64_t start;
	static int frames;
	static bool titled;
	const uint64_t now = SDL_GetPerformanceCounter(), frequency = SDL_GetPerformanceFrequency();

	if (!start)
		start = now;
	if (++frames < 2 || now - start < frequency / 2)
		return;
	fps_counted = (int)lround(frames * (double)frequency / (double)(now - start));
	start = now;
	frames = 0;
	if (console_settings.fps || titled)
	{
		char title[48];

		snprintf(title, sizeof(title), console_settings.fps ? TITLE " - %d FPS" : TITLE, fps_counted);
		SDL_SetWindowTitle(window, title);
		titled = console_settings.fps;
	}
}

/* The frames a second at the top right of the picture 'shown', its letters the game's pixels, with a shadow */
static void draw_fps(const SDL_Rect *shown)
{
	SDL_Rect cells[2][14 * 15];
	int count = 0;
	char text[16];
	const int pixel = SDL_max(1, shown->h / SIMPSONS_VIDEO_HEIGHT);
	int x;

	if (!console_settings.fps || fps_counted < 0)
		return;
	snprintf(text, sizeof(text), "%d FPS", SDL_min(fps_counted, 999));
	x = shown->x + shown->w - (4 * (int)strlen(text) + 3) * pixel;
	for (const char *letter = text; *letter; letter++, x += 4 * pixel)
	{
		const uint8_t *rows = fps_glyphs[strchr(FPS_GLYPHS, *letter) - FPS_GLYPHS];

		for (int row = 0; row < 5; row++)
			for (int column = 0; column < 3; column++)
				if (rows[row] & (4 >> column))
				{
					cells[1][count] = (SDL_Rect){ x + column * pixel, shown->y + (4 + row) * pixel, pixel, pixel };
					cells[0][count] = cells[1][count];
					cells[0][count].x += pixel;
					cells[0][count].y += pixel;
					count++;
				}
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRects(renderer, cells[0], count);
	SDL_SetRenderDrawColor(renderer, 255, 224, 0, 255);
	SDL_RenderFillRects(renderer, cells[1], count);
}

/* ---- the CONTROLS page's keys and buttons: Kenney's Input Prompts Pixel (CC0), 16 x 16 tiles, as tmnt-port's ---- */

typedef struct
{
	int what;      /* an SDL_Scancode or an SDL_GameControllerButton */
	uint8_t row, column, tiles;    /* where in the sheet, and how many tiles wide */
} prompt;

static const prompt key_prompts[] = {
	{ SDL_SCANCODE_ESCAPE, 0, 17, 1 }, { SDL_SCANCODE_F1, 0, 18, 1 }, { SDL_SCANCODE_F2, 0, 19, 1 },
	{ SDL_SCANCODE_F3, 0, 20, 1 }, { SDL_SCANCODE_F4, 0, 21, 1 }, { SDL_SCANCODE_F5, 0, 22, 1 },
	{ SDL_SCANCODE_F6, 0, 23, 1 }, { SDL_SCANCODE_F7, 0, 24, 1 }, { SDL_SCANCODE_F8, 0, 25, 1 },
	{ SDL_SCANCODE_F9, 0, 26, 1 }, { SDL_SCANCODE_F10, 0, 27, 1 }, { SDL_SCANCODE_F11, 0, 28, 1 },
	{ SDL_SCANCODE_F12, 0, 29, 1 },
	{ SDL_SCANCODE_1, 1, 17, 1 }, { SDL_SCANCODE_2, 1, 18, 1 }, { SDL_SCANCODE_3, 1, 19, 1 },
	{ SDL_SCANCODE_4, 1, 20, 1 }, { SDL_SCANCODE_5, 1, 21, 1 }, { SDL_SCANCODE_6, 1, 22, 1 },
	{ SDL_SCANCODE_7, 1, 23, 1 }, { SDL_SCANCODE_8, 1, 24, 1 }, { SDL_SCANCODE_9, 1, 25, 1 },
	{ SDL_SCANCODE_0, 1, 26, 1 }, { SDL_SCANCODE_MINUS, 1, 27, 1 }, { SDL_SCANCODE_EQUALS, 1, 29, 1 },
	{ SDL_SCANCODE_BACKSPACE, 1, 32, 2 },
	{ SDL_SCANCODE_Q, 2, 17, 1 }, { SDL_SCANCODE_W, 2, 18, 1 }, { SDL_SCANCODE_E, 2, 19, 1 },
	{ SDL_SCANCODE_R, 2, 20, 1 }, { SDL_SCANCODE_T, 2, 21, 1 }, { SDL_SCANCODE_Y, 2, 22, 1 },
	{ SDL_SCANCODE_U, 2, 23, 1 }, { SDL_SCANCODE_I, 2, 24, 1 }, { SDL_SCANCODE_O, 2, 25, 1 },
	{ SDL_SCANCODE_P, 2, 26, 1 }, { SDL_SCANCODE_LEFTBRACKET, 2, 27, 1 }, { SDL_SCANCODE_RIGHTBRACKET, 2, 28, 1 },
	{ SDL_SCANCODE_BACKSLASH, 2, 31, 1 },
	{ SDL_SCANCODE_A, 3, 18, 1 }, { SDL_SCANCODE_S, 3, 19, 1 }, { SDL_SCANCODE_D, 3, 20, 1 },
	{ SDL_SCANCODE_F, 3, 21, 1 }, { SDL_SCANCODE_G, 3, 22, 1 }, { SDL_SCANCODE_H, 3, 23, 1 },
	{ SDL_SCANCODE_J, 3, 24, 1 }, { SDL_SCANCODE_K, 3, 25, 1 }, { SDL_SCANCODE_L, 3, 26, 1 },
	{ SDL_SCANCODE_APOSTROPHE, 3, 27, 1 }, { SDL_SCANCODE_SEMICOLON, 3, 30, 1 },
	{ SDL_SCANCODE_RETURN, 3, 32, 2 }, { SDL_SCANCODE_KP_ENTER, 3, 32, 2 },
	{ SDL_SCANCODE_Z, 4, 19, 1 }, { SDL_SCANCODE_X, 4, 20, 1 }, { SDL_SCANCODE_C, 4, 21, 1 },
	{ SDL_SCANCODE_V, 4, 22, 1 }, { SDL_SCANCODE_B, 4, 23, 1 }, { SDL_SCANCODE_N, 4, 24, 1 },
	{ SDL_SCANCODE_M, 4, 25, 1 }, { SDL_SCANCODE_COMMA, 4, 26, 1 }, { SDL_SCANCODE_PERIOD, 4, 27, 1 },
	{ SDL_SCANCODE_SLASH, 4, 29, 1 },
	{ SDL_SCANCODE_UP, 4, 30, 1 }, { SDL_SCANCODE_RIGHT, 4, 31, 1 }, { SDL_SCANCODE_DOWN, 4, 32, 1 },
	{ SDL_SCANCODE_LEFT, 4, 33, 1 },
	{ SDL_SCANCODE_LALT, 5, 17, 2 }, { SDL_SCANCODE_RALT, 5, 17, 2 }, { SDL_SCANCODE_TAB, 5, 19, 2 },
	{ SDL_SCANCODE_DELETE, 5, 21, 2 }, { SDL_SCANCODE_END, 5, 23, 2 },
	{ SDL_SCANCODE_LCTRL, 6, 17, 2 }, { SDL_SCANCODE_RCTRL, 6, 17, 2 }, { SDL_SCANCODE_CAPSLOCK, 6, 19, 2 },
	{ SDL_SCANCODE_HOME, 6, 21, 2 }, { SDL_SCANCODE_PAGEUP, 6, 23, 2 }, { SDL_SCANCODE_PAGEDOWN, 6, 25, 2 },
	{ SDL_SCANCODE_SPACE, 6, 32, 2 },
	{ SDL_SCANCODE_LSHIFT, 7, 17, 2 }, { SDL_SCANCODE_RSHIFT, 7, 17, 2 }, { SDL_SCANCODE_INSERT, 7, 19, 2 },
	{ SDL_SCANCODE_PAUSE, 7, 25, 2 },
};

#if defined(__SWITCH__) || defined(SIMPSONS_SWITCH_PROMPTS)
/* The Switch's buttons by the letters on them. devkitPro's SDL has its game controller by the buttons' places, an
   Xbox controller's (its mapping: "a:b1,b:b0,x:b3,y:b2", libnx's buttons in their order, A B X Y): SDL's A, the lower
   one, is the Switch's B, its B the Switch's A, its X the Y and its Y the X; the shoulders L and R, the triggers ZL
   and ZR, Back and Start - and +. The sheet's white buttons (SIMPSONS_SWITCH_PROMPTS: a computer's check of them) */
static const prompt button_prompts[] = {
	{ SDL_CONTROLLER_BUTTON_A, 0, 14, 1 }, { SDL_CONTROLLER_BUTTON_B, 0, 13, 1 },
	{ SDL_CONTROLLER_BUTTON_X, 0, 16, 1 }, { SDL_CONTROLLER_BUTTON_Y, 0, 15, 1 },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, 1, 1, 1 }, { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 1, 2, 1 },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, 1, 3, 1 }, { SDL_CONTROLLER_BUTTON_DPAD_LEFT, 1, 4, 1 },
	{ SDL_CONTROLLER_BUTTON_BACK, 20, 13, 1 }, { SDL_CONTROLLER_BUTTON_START, 20, 14, 1 },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, 18, 15, 1 }, { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 18, 16, 1 },
	{ CONTROL_LEFT_TRIGGER, 18, 13, 1 }, { CONTROL_RIGHT_TRIGGER, 18, 14, 1 },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, 12, 16, 1 }, { SDL_CONTROLLER_BUTTON_RIGHTSTICK, 14, 16, 1 },
};
#else
static const prompt button_prompts[] = {
	{ SDL_CONTROLLER_BUTTON_A, 0, 4, 1 }, { SDL_CONTROLLER_BUTTON_B, 0, 5, 1 },
	{ SDL_CONTROLLER_BUTTON_X, 0, 6, 1 }, { SDL_CONTROLLER_BUTTON_Y, 0, 7, 1 },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, 1, 1, 1 }, { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 1, 2, 1 },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, 1, 3, 1 }, { SDL_CONTROLLER_BUTTON_DPAD_LEFT, 1, 4, 1 },
	{ SDL_CONTROLLER_BUTTON_BACK, 18, 4, 1 }, { SDL_CONTROLLER_BUTTON_START, 18, 5, 1 },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, 18, 7, 1 }, { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 18, 8, 1 },
	{ CONTROL_LEFT_TRIGGER, 18, 9, 1 }, { CONTROL_RIGHT_TRIGGER, 18, 10, 1 },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, 12, 16, 1 }, { SDL_CONTROLLER_BUTTON_RIGHTSTICK, 14, 16, 1 },
	{ SDL_CONTROLLER_BUTTON_GUIDE, 18, 2, 1 }, { SDL_CONTROLLER_BUTTON_MISC1, 18, 6, 1 },
};
#endif

#define PROMPT_SIZE  16
#define PROMPT_CLEAR 0xFFFF00FFu    /* magenta: the sheet's see-through */

static SDL_Surface *prompt_sheet;    /* input_prompts.bmp beside the executable, as ARGB */

#ifdef SIMPSONS_PROMPTS_EMBEDDED
/* (the sheet in the executable, ports/sdl2/prompts/embed.cmake: the Switch's games, an .nro or an .nsp's, have no
   file beside them) */
extern const unsigned char simpsons_prompts_bmp[];
extern const size_t simpsons_prompts_bmp_size;
#endif

static void load_prompts(const char *base)
{
	char path[1100];
	SDL_Surface *loaded;

#ifdef SIMPSONS_PROMPTS_EMBEDDED
	(void)base;
	snprintf(path, sizeof(path), "The executable's input prompts");
	loaded = SDL_LoadBMP_RW(SDL_RWFromConstMem(simpsons_prompts_bmp, (int)simpsons_prompts_bmp_size), 1);
#else
	snprintf(path, sizeof(path), "%sinput_prompts.bmp", base ? base : "");
	loaded = SDL_LoadBMP(path);
#endif
	if (!loaded)
	{
		fprintf(stderr, "%s cannot be read: CONTROLS shows the names alone\n", path);
		return;
	}
	prompt_sheet = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_FreeSurface(loaded);
}

static const prompt *find_prompt(const prompt *list, size_t count, int what)
{
	for (size_t index = 0; index < count; index++)
		if (list[index].what == what)
			return &list[index];
	return NULL;
}

/* One prompt into the picture, its top left at (x, y) (the board's x) */
static void draw_prompt(console_picture *into, const prompt *icon, int x, int y)
{
	const uint32_t *sheet = (const uint32_t *)prompt_sheet->pixels;
	const int pitch = prompt_sheet->pitch / 4;

	for (int row = 0; row < PROMPT_SIZE; row++)
		for (int column = 0; column < PROMPT_SIZE * icon->tiles; column++)
		{
			const uint32_t colour = sheet[(icon->row * PROMPT_SIZE + row) * pitch + icon->column * PROMPT_SIZE + column];

			if ((colour | 0xFF000000u) == PROMPT_CLEAR || y + row < 0 || y + row >= SIMPSONS_VIDEO_HEIGHT
				|| x + column < -into->left || x + column >= SIMPSONS_VIDEO_WIDTH + into->right)
				continue;
			into->pixels[(y + row) * into->stride + into->left + x + column] = colour & 0xFFFFFFu;
		}
}

/* On CONTROLS, each of the shown player's controls' keys and first button under its name */
static void draw_controls(console_picture *into)
{
	const control_binding *bindings = control_bindings[console_controls_player()];

	if (!prompt_sheet)
		return;
	for (int control = 0; control < CONTROLS; control++)
	{
		const prompt *icon, *first = NULL;
		int shown = 0, x, y;

		if (!console_controls_place(control, &x, &y))
			return;
#ifdef SIMPSONS_DEVICE_CONTROLS
		/* (a device's page: no keys, the control's buttons side by side) */
		for (int button = 0; button < CONTROL_BUTTONS; button++)
			if ((icon = find_prompt(button_prompts, SDL_arraysize(button_prompts), bindings[control].buttons[button]))
				!= NULL)
				draw_prompt(into, icon, x + shown++ * CONSOLE_CONTROLS_NEXT_X, y);
		(void)first;
		continue;
#endif
		for (int key = 0; key < CONTROL_KEYS && shown < 2; key++)
		{
			icon = find_prompt(key_prompts, SDL_arraysize(key_prompts), bindings[control].keys[key]);
			if (!icon || (first && first->row == icon->row && first->column == icon->column))
				continue;    /* (no picture, or the first key's: Enter and the keypad's) */
			draw_prompt(into, icon, x + (shown++ ? CONSOLE_CONTROLS_SECOND_X : 0), y);
			first = first ? first : icon;
		}
		if ((icon = find_prompt(button_prompts, SDL_arraysize(button_prompts), bindings[control].buttons[0])) != NULL)
			draw_prompt(into, icon, x + CONSOLE_CONTROLS_BUTTON_X, y);
	}
}
#endif

/* A frame a whole frame's time behind the game's pace (a screen a little slower than the board's 59.19 a second, or
   the machine busy) is not shown, so that it does not wait for the screen's refresh: the game, and its sound, keep
   their pace, a frame left out now and then (as tmnt-port's and outrunners-port's). Never two in a row, and none
   while frames go undrawn to keep the pace (late_pace, wait_for_frame): on a machine that draws no frame in its time
   every one was behind, and the picture stood still; there each frame drawn is shown, and fewer are drawn */
static bool frame_behind(void)
{
	return vsync_on && !left_out && late_pace == 0 && SDL_GetPerformanceCounter()
		> next_frame + SDL_GetPerformanceFrequency() * FRAME_RATE_DENOMINATOR / FRAME_RATE_NUMERATOR;
}

/* The window's picture, in the game's pixels: the board's 320, or the console's WIDE (console_wide_view: 358 for
   16:10, 398 for 16:9, 496 for 20:9) */
static int view_width(void)
{
#ifdef SIMPSONS_CONSOLE
	return console_wide_view();
#else
	return SIMPSONS_VIDEO_WIDTH;
#endif
}

/* The part of the texture the frame shows: the board's picture, or WIDE's wider one, its middle as wide as the
   window's picture; a narrower one (a screen WIDE leaves at the board's 320) between black bars */
static SDL_Rect content = { 0, 0, SIMPSONS_VIDEO_WIDTH, SIMPSONS_VIDEO_HEIGHT };

#if defined(__APPLE__) && TARGET_OS_IOS
/* ---- iOS and iPadOS (ports/ios) ---- */

static bool ios_background;    /* the app gone to the background (ios_app_events): nothing drawn there */

/* The app's Documents folder, which the Files app shows as the app's own (Info.plist's UIFileSharingEnabled and
   LSSupportsOpeningDocumentsInPlace): the device's folder, where the player puts the ROM set, and the folder of what
   the game keeps and of its log. HOME is the app's own container on iOS. The caller frees it (SDL_free) */
static char *ios_documents(void)
{
	const char *home = getenv("HOME");
	const size_t length = (home ? strlen(home) : 0) + sizeof("/Documents/");
	char *folder = SDL_malloc(length);

	if (folder)
		snprintf(folder, length, "%s/Documents/", home ? home : "");
	return folder;
}

/* The app going to the background and coming back, as SDL sends them (an event watch: at once, before the frame's
   events are read, as iOS may give the app no more time): from SDL_APP_WILLENTERBACKGROUND to
   SDL_APP_DIDENTERFOREGROUND nothing is drawn (present), an app's GPU work being refused there */
static int ios_app_events(void *data, SDL_Event *event)
{
	(void)data;
	if (event->type == SDL_APP_WILLENTERBACKGROUND)
		ios_background = true;
	else if (event->type == SDL_APP_DIDENTERFOREGROUND)
		ios_background = false;
	return 1;
}

/* The width of each of the screen's sides the notch or the Dynamic Island takes in landscape (the UIWindow's
   safeAreaInsets, the larger of left and right), in the renderer's pixels for a screen 'out_width' of them wide: the
   picture stays between them (present), and The Simpsons' WIDE is chosen for the width between them (follow_screen).
   Through the Objective-C runtime, from C. 0 where none is known */
static int ios_side_inset(int out_width)
{
#if defined(__arm64__) || defined(__aarch64__)
	typedef struct
	{
		double top, left, bottom, right;    /* (UIEdgeInsets: four CGFloats, doubles on a 64-bit iOS) */
	} edge_insets;
	SDL_SysWMinfo info;
	int points = 0, points_high = 0;
	edge_insets insets;

	SDL_VERSION(&info.version);
	if (!window || !SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_UIKIT || !info.info.uikit.window)
		return 0;
	SDL_GetWindowSize(window, &points, &points_high);
	if (points <= 0)
		return 0;
	/* (arm64 returns the structure in its registers: objc_msgSend as it is, there is no _stret there) */
	insets = ((edge_insets (*)(id, SEL))objc_msgSend)((id)info.info.uikit.window, sel_registerName("safeAreaInsets"));
	return (int)(SDL_max(insets.left, insets.right) * out_width / points + 0.5);
#else
	(void)out_width;
	return 0;    /* (an Intel Mac's Simulator: no notch to keep away from) */
#endif
}
#endif

#ifdef SIMPSONS_CONSOLE
/* WIDE AUTO (The Simpsons' default: OPTIONS > VIDEO > WIDE, Sebas: "Sí, AUTO por defecto"): the WIDE the screen's shape
   asks for, chosen again whenever the screen changes (console_set_screen_wide): a computer's monitor (the window's
   display, so the window is sized for it and full screen keeps the same WIDE; the full screen's size there), a
   device's screen, a phone's, a tablet's, a Fold opened or closed mid-game (tmnt-port's players: the picture stayed as
   it was, in half the screen), as tmnt-port's and outrunners-port's follow_screen. 20:9 where the screen is twice as
   wide as high or more (most phones: 18:9, 19.5:9, 20:9; an ultra wide monitor, 21:9; the picture as wide as the
   screen, a little short of its height), 16:9 from 16:9 on and on a tablet's touch screen (its short side
   TABLET_SIDE_INCHES or more: an open Fold's, the picture at the top and the touch controls under it), 16:10 from 3:2
   on, else the board's 320. (tmnt-port's says twice as wide, but asks for 20:9 itself: a 19.5:9 phone, 2340 x 1080,
   got 16:9 there, a tenth of its screen black at each side). A WIDE chosen by hand holds */
#define TABLET_SIDE_INCHES 4.0f

static int shape_width, shape_height;    /* the screen WIDE AUTO was last chosen for */

static bool tablet_screen(int width, int height)
{
#ifdef SIMPSONS_TOUCH
	float horizontal = 0.0f, vertical = 0.0f;

	if (!touch_display_dpi(NULL, &horizontal, &vertical) || horizontal <= 0.0f || vertical <= 0.0f)
		return false;
	return SDL_min(width / horizontal, height / vertical) >= TABLET_SIDE_INCHES;
#else
	(void)width;
	(void)height;
	return false;
#endif
}

static int wide_for_screen(int width, int height)
{
	const int long_side = SDL_max(width, height), short_side = SDL_min(width, height);

	if (long_side >= short_side * 2)
		return WIDE_20_9;
	if (long_side * 9 >= short_side * 16 || tablet_screen(width, height))
		return WIDE_16_9;
	return long_side * 2 >= short_side * 3 ? WIDE_16_10 : WIDE_OFF;
}

/* The screen's shape at a frame: the renderer's whole output on a device and full screen, else the desktop's mode of
   the display the window is on (the monitor's) */
static void screen_shape(int *width, int *height)
{
	SDL_DisplayMode mode;

	*width = *height = 0;
#if !defined(SIMPSONS_HANDHELD) && !defined(SIMPSONS_TOUCH)
	if (window && !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)
		&& SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) == 0)
	{
		*width = mode.w;
		*height = mode.h;
		return;
	}
#else
	(void)mode;
#endif
	if (renderer)
		SDL_GetRendererOutputSize(renderer, width, height);
}

/* WIDE AUTO's chosen again where the screen's shape changed (the log says so); the window sized again for it (by
   apply_settings, as a setting changed) */
static void follow_screen(void)
{
	int width, height;
	const int before = console_wide_in_effect();

	screen_shape(&width, &height);
#if defined(__APPLE__) && TARGET_OS_IOS
	width -= 2 * ios_side_inset(width);    /* (the screen between the notch's or the Dynamic Island's sides) */
#endif
	if (width <= 0 || height <= 0 || (width == shape_width && height == shape_height))
		return;
	shape_width = width;
	shape_height = height;
	console_set_screen_wide(wide_for_screen(width, height));
	fprintf(stderr, "Screen %dx%d: WIDE AUTO -> %s\n", width, height, console_wide_names[wide_for_screen(width, height)]);
	if (console_wide_in_effect() != before)
		console_settings.changed = true;
}
#endif

#ifdef SIMPSONS_TOUCH_CHECK
/* A computer's check of the touch screen's build (CMakeLists.txt's SIMPSONS_TOUCH_CHECK, SDL's dummy video): --screen
   WIDTHxHEIGHT is the phone's or the tablet's screen, and at each frame --video-dump names the whole screen as it is
   presented (the picture where WIDE and touch_layout put it, the touch controls over it) goes to screen_FRAME.bmp
   too */
static int check_screen_width, check_screen_height;

static void check_screen_dump(void)
{
	for (int index = 0; index < dump_count; index++)
		if (dump_frames[index] == simpsons_board.frame)
		{
			char path[1200];
			int width = 0, height = 0;
			SDL_Surface *screen;

			SDL_GetRendererOutputSize(renderer, &width, &height);
			screen = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
			if (!screen)
				return;
			snprintf(path, sizeof(path), "%s/screen_%" PRIu64 ".bmp", dump_dir, simpsons_board.frame);
			if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screen->pixels, screen->pitch) != 0
				|| SDL_SaveBMP(screen, path) != 0)
				fprintf(stderr, "%s: cannot be written (%s)\n", path, SDL_GetError());
			SDL_FreeSurface(screen);
			return;
		}
}
#endif

/* The picture in the window, as large as it fits with its proportions (a whole number of times its pixels with
   INTEGER SCALE), black around it */
static void present(void)
{
	const int kind = pixels_kind();
	const SDL_Rect source = content;
	const int view = view_width();
	int out_width = 0, out_height = 0;
	double scale;
	SDL_Rect shown;

	if (unseen)
		return;
#if defined(__APPLE__) && TARGET_OS_IOS
	if (ios_background)
		return;    /* (the app in the background: iOS refuses its GPU work there) */
#endif

	SDL_GetRendererOutputSize(renderer, &out_width, &out_height);
#ifdef SIMPSONS_CONSOLE
	follow_screen();    /* (WIDE AUTO: a Fold opened or closed, the window on another monitor: from the next frame) */
#endif
	scale = SDL_min((double)out_width / view, (double)out_height / SIMPSONS_VIDEO_HEIGHT);
	if (integer_scale() && scale >= 1.0)
		scale = floor(scale);    /* (every pixel of the game's the same size) */
	shown.w = (int)(source.w * scale);
	shown.h = (int)(SIMPSONS_VIDEO_HEIGHT * scale);
	shown.x = (out_width - shown.w) / 2;
	shown.y = (out_height - shown.h) / 2;
#if defined(__APPLE__) && TARGET_OS_IOS
	{
		/* (the picture between the sides the notch or the Dynamic Island takes: as wide as fits there) */
		const int room = out_width - 2 * ios_side_inset(out_width);

		if (room > 0 && (int)(view * scale) > room)
		{
			scale = (double)room / view;
			if (integer_scale() && scale >= 1.0)
				scale = floor(scale);
			shown.w = (int)(source.w * scale);
			shown.h = (int)(SIMPSONS_VIDEO_HEIGHT * scale);
			shown.x = (out_width - shown.w) / 2;
			shown.y = (out_height - shown.h) / 2;
		}
	}
#endif
#ifdef SIMPSONS_TOUCH
	{
		/* (a tall screen: the picture at the top, the touch controls in the room under it) */
		SDL_Rect frame = { (out_width - (int)(view * scale)) / 2, shown.y, (int)(view * scale), shown.h };

		touch_layout(out_width, out_height, &frame);
		shown.y = frame.y;
	}
#endif
	SDL_SetTextureScaleMode(texture, kind == 2 ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	if (kind == 1 || kind == 3)
	{
		const int times = SDL_max(1, (int)scale);
		const SDL_Rect whole = { 0, 0, source.w * times, SIMPSONS_VIDEO_HEIGHT * times };

		if (!scaled || scaled_times != times || scaled_width != source.w)
		{
			if (scaled)
				SDL_DestroyTexture(scaled);
			scaled = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
				source.w * times, SIMPSONS_VIDEO_HEIGHT * times);
			if (scaled)
				SDL_SetTextureScaleMode(scaled, SDL_ScaleModeLinear);
			scaled_times = times;
			scaled_width = source.w;
		}
		if (scaled)
		{
			SDL_SetRenderTarget(renderer, scaled);
			SDL_RenderCopy(renderer, texture, &source, &whole);
			SDL_SetRenderTarget(renderer, NULL);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scaled, &whole, &shown);
		}
		else
			SDL_RenderCopy(renderer, texture, &source, &shown);
		if (kind == 3)
		{
			make_crt_overlay(shown.w, shown.h);
			if (crt_overlay)
				SDL_RenderCopy(renderer, crt_overlay, NULL, &shown);
		}
	}
	else
		SDL_RenderCopy(renderer, texture, &source, &shown);
#ifdef SIMPSONS_CONSOLE
	count_fps();
	draw_fps(&shown);
#endif
#ifdef SIMPSONS_TOUCH
	touch_draw(renderer, out_width, out_height);
#endif
#ifdef SIMPSONS_TOUCH_CHECK
	check_screen_dump();
#endif
	left_out = frame_behind();
	if (!left_out)
	{
		SDL_RenderPresent(renderer);
		slow.shown++;
	}
}

#ifdef SIMPSONS_CONSOLE
/* The frame the video drew into 'into' (its rows SIMPSONS_VIDEO_MAX_WIDTH apart): WIDE's, with its sides, or the
   board's */
static void take_frame(console_picture *into)
{
	int width;
	const uint32_t *wide = simpsons_video_wide_frame(&width);

	if (width > SIMPSONS_VIDEO_WIDTH)
	{
		into->left = simpsons_video_wide_left();
		into->right = width - SIMPSONS_VIDEO_WIDTH - into->left;
		for (int y = 0; y < SIMPSONS_VIDEO_HEIGHT; y++)
			memcpy(&into->pixels[y * into->stride], &wide[y * SIMPSONS_VIDEO_MAX_WIDTH], (size_t)width * sizeof(uint32_t));
	}
	else
	{
		into->left = into->right = 0;
		for (int y = 0; y < SIMPSONS_VIDEO_HEIGHT; y++)
			memcpy(&into->pixels[y * into->stride], &game_frame()->pixels[y * SIMPSONS_VIDEO_WIDTH],
				SIMPSONS_VIDEO_WIDTH * sizeof(uint32_t));
	}
}

/* The picture into the texture, and the part of it the window shows: WIDE's middle as wide as the window's picture,
   or the board's 320 */
static void upload_picture(const console_picture *shown_picture)
{
	const int width = SIMPSONS_VIDEO_WIDTH + shown_picture->left + shown_picture->right;
	const SDL_Rect place = { 0, 0, width, SIMPSONS_VIDEO_HEIGHT };
	const int view = view_width();

	SDL_UpdateTexture(texture, &place, shown_picture->pixels, shown_picture->stride * (int)sizeof(uint32_t));
	content = place;
	if (width > view)
	{
		content.x = (width - view) / 2;
		content.w = view;
	}
}
#endif

/* The frame the video drew, with the console's picture over it, into the window */
static void show_frame(void)
{
#ifdef SIMPSONS_CONSOLE
	take_frame(&picture);
	console_overlay(&picture);
	draw_controls(&picture);    /* (OPTIONS > CONTROLS: the keys' and buttons' pictures) */
	upload_picture(&picture);
#else
	memcpy(picture_pixels, game_frame()->pixels, sizeof(picture_pixels));
	SDL_UpdateTexture(texture, NULL, picture_pixels, SIMPSONS_VIDEO_WIDTH * (int)sizeof(uint32_t));
#endif
	present();
}

static void toggle_full_screen(void)
{
	const bool full = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef __SWITCH__
	(void)full;
	return;    /* (a keyboard's F11 on the Switch: its screen is the window already, apply_settings) */
#endif
	SDL_SetWindowFullscreen(window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
#ifdef SIMPSONS_CONSOLE
	console_settings.full_screen = full;
	console_settings.changed = true;
	launcher_full_screen = false;    /* (the player's own choice from here: kept) */
#endif
}

static void next_pixels(void)
{
#ifdef SIMPSONS_CONSOLE
	console_settings.pixels = (console_settings.pixels + 1) % PIXELS_KINDS;
	console_settings.changed = true;
#else
	arcade_pixels = (arcade_pixels + 1) % PIXEL_KINDS;
#endif
}

/* ---- the sound ---- */

static void open_audio(void)
{
	SDL_AudioSpec wanted, got;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
		return;
	SDL_zero(wanted);
	wanted.freq = (int)game_audio_rate();
	wanted.format = AUDIO_S16SYS;
	wanted.channels = 2;
	wanted.samples = 1024;
	audio = SDL_OpenAudioDevice(NULL, 0, &wanted, &got, 0);
	if (audio)
		SDL_PauseAudioDevice(audio, 0);
}

/* the frame's samples (stereo pairs), unless the queue is already many frames ahead (a slow start, a pause); in The
   Simpsons at OPTIONS' volume (5: the game's own; 10 twice it, clipped; 0 while the console keeps it silent) */
static void queue_sound(void)
{
	size_t count;
	const int16_t *samples = game_audio(&count);
	const size_t values = count * 2u, bytes = values * sizeof(int16_t);

	if (!audio || !count || fast)
		return;
	if (SDL_GetQueuedAudioSize(audio) == 0)
		slow.sound_empty++;    /* (the sound ran out: a gap heard; --slow's report) */
	if (SDL_GetQueuedAudioSize(audio) > QUEUE_FRAMES_MAX * bytes)
		return;
#ifdef SIMPSONS_CONSOLE
	const int volume = console_volume();

	if (volume != 5)
	{
		static int16_t loud[16384];
		const size_t taken = values < SDL_arraysize(loud) ? values : SDL_arraysize(loud);

		for (size_t index = 0; index < taken; index++)
		{
			const int value = samples[index] * volume / 5;

			loud[index] = (int16_t)(value > 32767 ? 32767 : (value < -32768 ? -32768 : value));
		}
		SDL_QueueAudio(audio, loud, (Uint32)(taken * sizeof(int16_t)));
		return;
	}
#endif
	SDL_QueueAudio(audio, samples, (Uint32)bytes);
}

/* ---- the game controllers ---- */

static bool stick(SDL_GameController *pad, SDL_GameControllerAxis axis, int sign)
{
	return pad && SDL_GameControllerGetAxis(pad, axis) * sign > STICK_DEAD_ZONE;
}

#ifdef SIMPSONS_CONTROLLER_DB
/* SDL_GameControllerDB's mappings for this platform (ports/sdl2/controllerdb, built in by CMakeLists.txt) */
extern const unsigned char simpsons_controller_db[];
extern const size_t simpsons_controller_db_size;
#endif

/* Text added at the end of a growing one */
static void add_text(char **text, size_t *length, const char *more, size_t count)
{
	char *grown = (char *)realloc(*text, *length + count + 1);

	if (!grown)
		return;
	memcpy(grown + *length, more, count);
	*length += count;
	grown[*length] = '\0';
	*text = grown;
}

static char *saves_folder(void);

#ifdef SIMPSONS_HANDHELD_KEYS
/* Anbernic's H700 handhelds: their buttons, a joystick SDL 2.0.12 (the stock firmware's) does not know, as a game
   controller's (tmnt-port's and outrunners-port's mapping); MENU, its button 8, leaves the game as the firmware's other
   games */
#define HANDHELD_MENU_BUTTON 8

static const char *const device_mappings[] = {
	"19000000010000000100000000010000,ANBERNIC-keys,a:b0,b:b1,x:b3,y:b2,leftshoulder:b4,rightshoulder:b10,"
	"lefttrigger:b5,righttrigger:b11,back:b6,start:b7,guide:b8,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,"
	"leftx:a0,lefty:a1,rightx:a2,righty:a3,leftstick:b9,rightstick:b12,platform:Linux,",
};
#endif

#ifndef SIMPSONS_HANDHELD    /* (a device's controls are its own) */
/* gamecontrollerdb.txt, in the folder of what the game keeps or else beside the executable: the mappings of the
   game controllers SDL does not know (or knows wrong), a line each as SDL_GameControllerDB has them; its lines for
   this platform (or for none) at the end of 'text', without carriage returns */
static void add_mappings_file(char **text, size_t *length)
{
	char path[1024];
	char *folders[2];
	const char *platform = SDL_GetPlatform();

	folders[0] = saves_folder();
	folders[1] = SDL_GetBasePath();
	for (int folder = 0; folder < 2; folder++)
	{
		FILE *file;
		char line[2048];
		int lines = 0;

		if (!folders[folder])
			continue;
		snprintf(path, sizeof(path), "%s" MAPPINGS_FILE, folders[folder]);
		if ((file = fopen(path, "r")) == NULL)
			continue;
		while (fgets(line, sizeof(line), file))
		{
			char *end = line + strlen(line), *field = strstr(line, "platform:");

			while (end > line && (end[-1] == '\n' || end[-1] == '\r'))
				*--end = '\0';
			if (line[0] == '\0' || line[0] == '#'
				|| (field && (strncmp(field + 9, platform, strlen(platform)) != 0 || field[9 + strlen(platform)] != ',')))
				continue;
			add_text(text, length, line, strlen(line));
			add_text(text, length, "\n", 1);
			lines++;
		}
		fclose(file);
		fprintf(stderr, "Game controllers: %s, %d mappings for %s\n", path, lines, platform);
		break;
	}
	SDL_free(folders[0]);
	SDL_free(folders[1]);
}
#endif

/* The game controllers' mappings, a hint before SDL starts (SDL_HINT_GAMECONTROLLERCONFIG, which SDL reads before
   it looks for joysticks: a mapping added after SDL_Init leaves a joystick already plugged in a joystick, which the
   game does not take). SDL_GameControllerDB's for this platform built in, the environment's SDL_GAMECONTROLLERCONFIG
   (Steam's, in the games it starts), and last the player's gamecontrollerdb.txt, so that its lines go over the others
   for the same controller. The hint overrides the environment's, which SDL_GetHint takes over a hint of normal
   priority: the built-in lines and the file left out (tmnt-port's) */
static void add_mappings(void)
{
	char *text = NULL;
	size_t length = 0;
	const char *environment = getenv(SDL_HINT_GAMECONTROLLERCONFIG);

#ifdef SIMPSONS_CONTROLLER_DB
	add_text(&text, &length, (const char *)simpsons_controller_db, simpsons_controller_db_size);
#endif
#ifdef SIMPSONS_HANDHELD_KEYS
	for (size_t mapping = 0; mapping < SDL_arraysize(device_mappings); mapping++)
	{
		add_text(&text, &length, device_mappings[mapping], strlen(device_mappings[mapping]));
		add_text(&text, &length, "\n", 1);
	}
#endif
	if (environment && environment[0])
	{
		add_text(&text, &length, environment, strlen(environment));
		add_text(&text, &length, "\n", 1);
		fprintf(stderr, "Game controllers: the environment's " SDL_HINT_GAMECONTROLLERCONFIG " too\n");
	}
#ifndef SIMPSONS_HANDHELD
	add_mappings_file(&text, &length);
#endif
	if (text)
		SDL_SetHintWithPriority(SDL_HINT_GAMECONTROLLERCONFIG, text, SDL_HINT_OVERRIDE);
	free(text);
}

/* What SDL finds of each joystick, in the log (a player's report on tmnt-port: on an Nvidia Shield the controllers'
   buttons did what they should not): its name, its GUID, whether it is a game controller and its mapping; and the
   first presses of keys and buttons, to tell a controller's button that comes as a key */
#define INPUT_LOG_PRESSES   48

static void log_joystick(int index)
{
	char guid_text[33];
	const SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(index);
	const char *name = SDL_JoystickNameForIndex(index);
	char *mapping;

	SDL_JoystickGetGUIDString(guid, guid_text, sizeof(guid_text));
	mapping = SDL_GameControllerMappingForGUID(guid);
	fprintf(stderr, "Joystick %d: %s, GUID %s, %s%s\n", index, name ? name : "(no name)", guid_text,
		SDL_IsGameController(index) ? "a game controller: " : "no game controller (no mapping)",
		SDL_IsGameController(index) && mapping ? mapping : "");
	SDL_free(mapping);
}

static void log_press(const SDL_Event *event)
{
	static int presses;

	if (presses >= INPUT_LOG_PRESSES)
		return;
	if (event->type == SDL_KEYDOWN && !event->key.repeat)
		fprintf(stderr, "Key %s (scancode %d)\n", SDL_GetKeyName(event->key.keysym.sym), (int)event->key.keysym.scancode);
	else if (event->type == SDL_CONTROLLERBUTTONDOWN)
		fprintf(stderr, "Game controller %d: %s\n", (int)event->cbutton.which,
			SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event->cbutton.button));
	else if (event->type == SDL_JOYBUTTONDOWN)
		fprintf(stderr, "Joystick %d: button %d\n", (int)event->jbutton.which, (int)event->jbutton.button);
	else
		return;
	if (++presses == INPUT_LOG_PRESSES)
		fprintf(stderr, "(no more presses in the log)\n");
}

/* A TV's remote, which SDL takes for a game controller (the Shield's, Android TV's, Fire TV's: "... Remote"); on
   Android also one by its buttons, whatever its name: a pad, OK (A) and back (Back), but no B and no Start */
static bool is_remote(SDL_GameController *pad)
{
	const char *name = SDL_GameControllerName(pad);

	for (const char *at = name ? name : ""; *at; at++)
		if (SDL_strncasecmp(at, "remote", 6) == 0)
			return true;
#ifdef __ANDROID__
	if (!SDL_GameControllerHasButton(pad, SDL_CONTROLLER_BUTTON_B)
		&& !SDL_GameControllerHasButton(pad, SDL_CONTROLLER_BUTTON_START))
		return true;
#endif
	return false;
}

#if defined(__ANDROID__) && defined(SIMPSONS_CONSOLE)
/* The game controller 'which' is a TV's remote: its Back is its player's JUMP, not the pause (device_controls) */
static bool remote_back(SDL_JoystickID which)
{
	SDL_GameController *pad = SDL_GameControllerFromInstanceID(which);

	return pad && is_remote(pad);
}
#else
#define remote_back(which) false
#endif

/* A game controller plugged in: the first player without one, but a game controller before a TV's remote (a
   player's report on tmnt-port: the Shield's remote, found first, took player 1 from his Xbox controller) */
static void controller_added(int which)
{
	for (int player = 0; player < PLAYERS; player++)
		if (!controllers[player])
		{
			controllers[player] = SDL_GameControllerOpen(which);
			if (!controllers[player] || is_remote(controllers[player]))
				return;
			for (int before = 0; before < player; before++)
				if (is_remote(controllers[before]))
				{
					SDL_GameController *remote = controllers[before];

					controllers[before] = controllers[player];
					controllers[player] = remote;
					return;
				}
			return;
		}
}

static void controller_removed(SDL_JoystickID which)
{
	for (int player = 0; player < PLAYERS; player++)
		if (controllers[player] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[player])) == which)
		{
			SDL_GameControllerClose(controllers[player]);
			controllers[player] = NULL;
		}
}

/* ---- the end ---- */

#ifndef SIMPSONS_CONSOLE
static void save_eeprom(void);
#endif

static void quit(void)
{
#ifndef SIMPSONS_CONSOLE
	save_eeprom();
#endif
	if (hashes)
		fclose(hashes);
	if (video_hashes)
		fclose(video_hashes);
	if (sound_hashes)
		fclose(sound_hashes);
	simpsons_script_free(script);
	if (audio)
		SDL_CloseAudioDevice(audio);
	if (crt_overlay)
		SDL_DestroyTexture(crt_overlay);
	if (scaled)
		SDL_DestroyTexture(scaled);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
#ifdef __SWITCH__
	romfsExit();
#endif
	exit(0);
}

#ifdef SIMPSONS_CONSOLE
/* ---- The Simpsons: each player's controls as OPTIONS > CONTROLS binds them (controls.c), the left stick moving
   besides; the menus' fixed keys besides (the arrows, Enter, Escape or Backspace), whatever the players' are ---- */

#define TRIGGER_BOUND  16384    /* a trigger takes the control CONTROLS waits for from halfway down (of 32767) */

static uint32_t actions_pressed;      /* the menus' fixed keys newly pressed since the last frame */
static bool starts_held[PLAYERS];     /* each player's START at the last frame */
static bool pause_requested;          /* Escape, a game controller's Back or a player's START in a game: run_pause,
                                         once the frame is done */

static void action_key(SDL_Keycode key)
{
	switch (key)
	{
		case SDLK_UP: actions_pressed |= ACTION_BIT(ACTION_UP); break;
		case SDLK_DOWN: actions_pressed |= ACTION_BIT(ACTION_DOWN); break;
		case SDLK_LEFT: actions_pressed |= ACTION_BIT(ACTION_LEFT); break;
		case SDLK_RIGHT: actions_pressed |= ACTION_BIT(ACTION_RIGHT); break;
		case SDLK_ESCAPE: case SDLK_BACKSPACE: CASE_AC_BACK actions_pressed |= ACTION_BIT(ACTION_BACK); break;
		case SDLK_RETURN: case SDLK_KP_ENTER: actions_pressed |= ACTION_BIT(ACTION_START); break;
		default: break;
	}
}

/* The button or trigger pressed on game controller 'which' for the control CONTROLS waits for, when it is the shown
   player's */
static void bind_button(SDL_JoystickID which, SDL_GameControllerButton pressed)
{
	const int player = console_controls_player();

	if (player < PLAYERS && controllers[player]
		&& SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[player])) == which)
	{
		controls_bind_button(player, console_controls_waiting(), pressed);
		console_controls_bound();
	}
#ifdef SIMPSONS_DEVICE_CONTROLS
	else
		console_controls_bound();    /* (another player's controller: the control as it was. A device has no Escape
		                                for it, and the shown player may have no controller: the page would wait for
		                                ever) */
#endif
}
#endif

static void read_events(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		log_press(&event);
#ifdef SIMPSONS_TOUCH
		touch_event(&event);
#endif
		switch (event.type)
		{
			case SDL_QUIT:
				quit();
				return;
#if defined(__ANDROID__) && defined(SIMPSONS_CONSOLE)
			case SDL_APP_WILLENTERBACKGROUND:
				if (console_can_pause())
					pause_requested = true;    /* (Android may end the game there: a game waits, paused) */
				break;
#endif
#if defined(__APPLE__) && TARGET_OS_IOS && defined(SIMPSONS_CONSOLE)
			case SDL_APP_WILLENTERBACKGROUND:
				if (console_can_pause())
					pause_requested = true;    /* (the home gesture, a call: a game waits, paused) */
				break;
#endif
#ifdef SIMPSONS_HANDHELD_KEYS
			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.button == HANDHELD_MENU_BUTTON)
				{
					quit();    /* (MENU leaves the game, as the firmware's other games) */
					return;
				}
				break;
#endif
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break;
#if defined(SIMPSONS_HANDHELD_KEYS) && defined(SIMPSONS_CONSOLE)
				if (event.key.keysym.sym == SDLK_VOLUMEUP || event.key.keysym.sym == SDLK_VOLUMEDOWN)
				{
					/* (the handheld's volume keys: OPTIONS > VOLUME's, kept) */
					const int volume = console_settings.volume + (event.key.keysym.sym == SDLK_VOLUMEUP ? 1 : -1);

					console_settings.volume = SDL_clamp(volume, 0, 10);
					console_settings.changed = true;
					break;
				}
#endif
#ifdef SIMPSONS_CONSOLE
				if (console_controls_waiting() >= 0)
				{
					/* CONTROLS waits for a key: this one (Escape leaves the control as it was) */
					if (event.key.keysym.sym != SDLK_ESCAPE)
						controls_bind_key(console_controls_player(), console_controls_waiting(), event.key.keysym.scancode);
					console_controls_bound();
					break;
				}
				if (!(event.key.keysym.mod & KMOD_ALT))    /* (Alt+Enter: the full screen alone) */
					action_key(event.key.keysym.sym);
				if ((event.key.keysym.sym == SDLK_ESCAPE || (event.key.keysym.sym == SDLK_AC_BACK && AC_BACK_IS_BACK)) && console_can_pause())
					pause_requested = true;
#else
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit();
					return;
				}
#endif
				if (event.key.keysym.sym == SDLK_F11
					|| (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_ALT)))
					toggle_full_screen();
				if (event.key.keysym.sym == SDLK_F3)
					next_pixels();
				break;
#ifdef SIMPSONS_CONSOLE
			case SDL_CONTROLLERBUTTONDOWN:
				/* ... or a button of the game controller plugged in for that player */
				if (console_controls_waiting() >= 0)
					bind_button(event.cbutton.which, (SDL_GameControllerButton)event.cbutton.button);
				else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK && console_can_pause()
					&& !remote_back(event.cbutton.which))
					pause_requested = true;    /* (a TV remote's: its home pauses, SDL_APP_WILLENTERBACKGROUND) */
				break;
			case SDL_CONTROLLERAXISMOTION:
				/* ... or one of its triggers, pressed halfway */
				if (console_controls_waiting() >= 0 && event.caxis.value >= TRIGGER_BOUND
					&& (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
					bind_button(event.caxis.which, event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT
						? CONTROL_LEFT_TRIGGER : CONTROL_RIGHT_TRIGGER);
				break;
#endif
			case SDL_JOYDEVICEADDED:
				log_joystick(event.jdevice.which);
				break;
			case SDL_CONTROLLERDEVICEADDED:
				controller_added(event.cdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				controller_removed(event.cdevice.which);
				break;
			default:
				break;
		}
	}
}

/* Each player's stick and buttons as controls.c binds them (simpsons/machine.h's bits, set when pressed): his keys,
   his game controller's buttons and its left stick */
static uint8_t player_pressed(int player)
{
	SDL_GameController *pad = controllers[player];
	uint8_t pressed = 0;

	pressed |= controls_holds(player, pad, CONTROL_LEFT) || stick(pad, SDL_CONTROLLER_AXIS_LEFTX, -1) ? SIMPSONS_LEFT : 0;
	pressed |= controls_holds(player, pad, CONTROL_RIGHT) || stick(pad, SDL_CONTROLLER_AXIS_LEFTX, 1) ? SIMPSONS_RIGHT : 0;
	pressed |= controls_holds(player, pad, CONTROL_UP) || stick(pad, SDL_CONTROLLER_AXIS_LEFTY, -1) ? SIMPSONS_UP : 0;
	pressed |= controls_holds(player, pad, CONTROL_DOWN) || stick(pad, SDL_CONTROLLER_AXIS_LEFTY, 1) ? SIMPSONS_DOWN : 0;
	pressed |= controls_holds(player, pad, CONTROL_ATTACK) ? SIMPSONS_ATTACK : 0;
	pressed |= controls_holds(player, pad, CONTROL_JUMP) ? SIMPSONS_JUMP : 0;
	return pressed;
}

#ifdef SIMPSONS_CONSOLE
/* ---- --actions: the menus' actions and the players' own, by frame ---- */

#define MAX_ACTIONS     4096
#define KIND_PAUSE      ACTIONS          /* (past the menus' actions) */
#define KIND_DUMP       (ACTIONS + 1)

static struct
{
	uint64_t frame;
	int kind;        /* ACTION_*, KIND_PAUSE or KIND_DUMP */
	int player;
	bool own;        /* (a player given: up ... back are that player's stick and buttons, not the menus' keys) */
	int button;      /* (KIND_PAD's: the game controller's button, or a trigger as controls.h has them) */
} actions[MAX_ACTIONS];
static size_t action_count, next_action;

#ifdef SIMPSONS_PAD_CHECK
/* --pads N (a computer's check, SIMPSONS_PAD_CHECK: never a player's build): N game controllers of SDL's own making
   (its virtual joysticks), plugged in as the game starts, the players' in turn; and --actions' "FRAME pad BUTTON
   PLAYER" presses that button of the player's controller for PAD_FRAMES frames (SDL's names: a, b, x, y, back, start,
   leftshoulder, rightshoulder, leftstick, rightstick, dpup, dpdown, dpleft, dpright, lefttrigger, righttrigger). So
   CONTROLS binds a controller's buttons, and the port reads them, with none plugged in: with --pads a scripted run
   reads the players' controls too (read_players) */
#define KIND_PAD        (ACTIONS + 2)
#define PAD_FRAMES      3

static int check_pads;
static struct
{
	int button, frames;    /* the button held and the frames it has left */
	bool rested;           /* its triggers let go: SDL's virtual axes start halfway */
} pads[PLAYERS];

static void pad_set(int player, int button, bool down)
{
	SDL_Joystick *joystick = controllers[player] ? SDL_GameControllerGetJoystick(controllers[player]) : NULL;

	if (!joystick)
		fprintf(stderr, "pad: player %d has no game controller\n", player + 1);
	else if (button == CONTROL_LEFT_TRIGGER || button == CONTROL_RIGHT_TRIGGER)
		SDL_JoystickSetVirtualAxis(joystick, button == CONTROL_LEFT_TRIGGER ? SDL_CONTROLLER_AXIS_TRIGGERLEFT
			: SDL_CONTROLLER_AXIS_TRIGGERRIGHT, down ? SDL_JOYSTICK_AXIS_MAX : SDL_JOYSTICK_AXIS_MIN);
	else
		SDL_JoystickSetVirtualButton(joystick, button, down ? 1 : 0);
}

/* Each frame: the buttons held their frames let go, a controller's triggers at rest once it is there */
static void pads_frame(void)
{
	for (int player = 0; player < PLAYERS; player++)
	{
		if (controllers[player] && !pads[player].rested && check_pads > 0)
		{
			pad_set(player, CONTROL_LEFT_TRIGGER, false);
			pad_set(player, CONTROL_RIGHT_TRIGGER, false);
			pads[player].rested = true;
		}
		if (pads[player].frames > 0 && --pads[player].frames == 0)
			pad_set(player, pads[player].button, false);
	}
}

static void pad_press(int player, int button)
{
	if (pads[player].frames > 0)
		pad_set(player, pads[player].button, false);
	pads[player].button = button;
	pads[player].frames = PAD_FRAMES;
	pad_set(player, button, true);
}
#endif

static const char *load_actions(const char *path)
{
	static const char *const names[ACTIONS + 2] = { "up", "down", "left", "right", "accept", "back", "start", "pause",
		"dump" };
	char line[256], name[32];
	unsigned long long frame;
	int player;
	FILE *input = fopen(path, "r");

	if (!input)
		return "The actions file cannot be read.";
	while (fgets(line, sizeof(line), input) && action_count < MAX_ACTIONS)
	{
		int kind, read;

#ifdef SIMPSONS_PAD_CHECK
		{
			char button[32];

			if (line[0] != '#' && (read = sscanf(line, "%llu %31s %31s %d", &frame, name, button, &player)) >= 3
				&& strcmp(name, "pad") == 0)
			{
				const int which = strcmp(button, "lefttrigger") == 0 ? (int)CONTROL_LEFT_TRIGGER
					: (strcmp(button, "righttrigger") == 0 ? (int)CONTROL_RIGHT_TRIGGER
						: (int)SDL_GameControllerGetButtonFromString(button));

				if (which < 0)
				{
					fclose(input);
					return "A pad's button is not one of SDL's names (a, b, x, y, back, start, leftshoulder...).";
				}
				actions[action_count].frame = frame;
				actions[action_count].kind = KIND_PAD;
				actions[action_count].player = read == 4 && player >= 1 && player <= PLAYERS ? player - 1 : 0;
				actions[action_count].own = true;
				actions[action_count].button = which;
				action_count++;
				continue;
			}
		}
#endif
		if (line[0] == '#' || (read = sscanf(line, "%llu %31s %d", &frame, name, &player)) < 2)
			continue;
		for (kind = 0; kind < ACTIONS + 2 && strcmp(name, names[kind]) != 0; kind++)
			;
		if (kind == ACTIONS + 2)
		{
			fclose(input);
			return "An action is not one of up, down, left, right, accept, back, start, pause, dump.";
		}
		actions[action_count].frame = frame;
		actions[action_count].kind = kind;
		actions[action_count].player = read == 3 && player >= 1 && player <= PLAYERS ? player - 1 : 0;
		actions[action_count].own = read == 3;
		action_count++;
	}
	fclose(input);
	return NULL;
}

/* The frame's actions into the players' controls and STARTs and the menus' keys; a pause (asked for, or a START that
   pauses) leaves the frame's next actions to the pause */
static void scripted_actions(uint8_t players[PLAYERS], uint8_t *starts)
{
	static const uint8_t controls[ACTIONS] = { SIMPSONS_UP, SIMPSONS_DOWN, SIMPSONS_LEFT, SIMPSONS_RIGHT,
		SIMPSONS_ATTACK, SIMPSONS_JUMP, 0 };
	const uint64_t frame = frames_before + simpsons_board.frame;

#ifdef SIMPSONS_PAD_CHECK
	pads_frame();
#endif
	while (next_action < action_count && actions[next_action].frame < frame)
		next_action++;    /* (a pause's that it did not take) */
	while (next_action < action_count && actions[next_action].frame == frame)
	{
		const int kind = actions[next_action].kind, player = actions[next_action].player;
		const bool own = actions[next_action].own;

		next_action++;
#ifdef SIMPSONS_PAD_CHECK
		if (kind == KIND_PAD)
		{
			pad_press(player, actions[next_action - 1].button);
			continue;
		}
#endif
		if (kind == KIND_PAUSE)
		{
			if (console_can_pause())
			{
				pause_requested = true;
				return;
			}
			continue;
		}
		if (kind == KIND_DUMP)
			continue;    /* (the pause's: out of it, --video-dump) */
		if (kind == ACTION_START)
		{
			*starts |= (uint8_t)(1u << player);
			if (console_start_pauses(player))
				return;
			continue;
		}
		if (own)
			players[player] |= controls[kind];
		else
			actions_pressed |= ACTION_BIT(kind);
	}
}

/* The pause's next scripted action at its frame (the one it came at: a LOAD STATE changes the board's): the pause's
   own (up, down, accept; back or start go on), and whether a dump of its picture comes first */
#define PAUSE_BACK  ACTION_BIT(ACTION_BACK)

static uint64_t pause_frame;    /* the frame the pause came at */

static uint32_t scripted_pause_action(bool *dump)
{
	int kind;

	*dump = false;
	if (next_action >= action_count || actions[next_action].frame != pause_frame)
		return 0;
	kind = actions[next_action++].kind;
	if (kind == KIND_DUMP)
	{
		*dump = true;
		return 0;
	}
	if (kind == ACTION_UP || kind == ACTION_DOWN || kind == ACTION_ACCEPT)
		return ACTION_BIT(kind);
	return kind == ACTION_BACK || kind == ACTION_START ? PAUSE_BACK : 0;
}

/* ---- The Simpsons' controls into the console, which puts each player's on his character's slot ---- */

#if defined(SIMPSONS_TOUCH) || defined(__ANDROID__)
/* What a device adds to a player's controls ('pressed', and his START held): the touch screen's are player 1's
   (touch.c); a TV's remote (Android's: the Chromecast with Google TV's, a pad, OK and back alone) has its back as JUMP
   (the menus' back, the pause's CONTINUE) and its OK, ATTACK, as the START of a player out of the game too (in, or a
   continue), as tmnt-port's */
static void device_controls(int player, uint8_t *pressed, bool *start)
{
#ifdef SIMPSONS_TOUCH
	if (player == 0)
	{
		*pressed |= touch_controls();
		*start = *start || touch_start();
	}
#endif
#ifdef __ANDROID__
	if (controllers[player] && is_remote(controllers[player]))
	{
		if (SDL_GameControllerGetButton(controllers[player], SDL_CONTROLLER_BUTTON_BACK))
			*pressed |= SIMPSONS_JUMP;
		if (SDL_GameControllerGetButton(controllers[player], SDL_CONTROLLER_BUTTON_A) && console_can_pause()
			&& !console_start_pauses(player))
			*start = true;
	}
#endif
}
#endif

/* Each player's controls as CONTROLS binds them, and their START newly pressed (while CONTROLS waits for a key or
   button, none) */
static void read_players(uint8_t players[PLAYERS], uint8_t *starts)
{
	const bool waiting = console_controls_waiting() >= 0;
#ifdef SIMPSONS_PAD_CHECK
	const bool unread = scripted && check_pads == 0;    /* (--pads: its controllers' buttons are the players') */
#else
	const bool unread = scripted;
#endif

	*starts = 0;
	for (int player = 0; player < PLAYERS; player++)
	{
		bool start = !unread && controls_holds(player, controllers[player], CONTROL_START);
		uint8_t pressed = waiting || unread ? 0 : player_pressed(player);

#if defined(SIMPSONS_TOUCH) || defined(__ANDROID__)
		if (!scripted)
			device_controls(player, &pressed, &start);    /* (the touch screen's, a TV remote's) */
#endif
		if (start && !starts_held[player] && !waiting)
			*starts |= (uint8_t)(1u << player);
		starts_held[player] = start;
		players[player] = waiting ? 0 : pressed;
	}
}

static void read_controls(void)
{
	uint8_t players[PLAYERS], starts = 0;

#ifdef SIMPSONS_PAD_CHECK
	const bool from_script = script && check_pads == 0;    /* (--pads: its controllers', read_players) */
#else
	const bool from_script = script != NULL;
#endif

	if (console_controls_defaults())
		controls_defaults(console_controls_player());
	if (from_script)
		for (int player = 0; player < PLAYERS; player++)
			players[player] = (uint8_t)~simpsons_board.players[player];    /* (the script's, on the board) */
	else
		read_players(players, &starts);
	scripted_actions(players, &starts);
	for (int player = 0; player < PLAYERS; player++)
		if ((starts & (1u << player)) && console_start_pauses(player))
		{
			starts &= (uint8_t)~(1u << player);
			pause_requested = true;    /* (START of a player in the game: the pause, as a console's) */
		}
	console_actions(console_controls_waiting() >= 0 ? 0 : actions_pressed);
	console_controls(players, starts);
	actions_pressed = 0;
}

/* MULTIPLAYER's game: this device's player's controls (player 1's keys and game controller, or --actions' player 1)
   and START go to the network, and every device's come back onto the board (console_lan_frame), nothing of the port's
   own keys in the game; till they have come the frame holds: the last picture again (WAITING FOR its device's name),
   the events read, the pace kept. Escape or Back leaves the game (as tmnt-port's) */
static void show_frame(void);
static void wait_for_frame(void);

static void lan_controls(void)
{
	uint8_t players[PLAYERS], starts = 0;

	read_players(players, &starts);
	scripted_actions(players, &starts);
	actions_pressed = 0;
	while (!console_lan_frame(players[0], (starts & 1u) != 0))
	{
		read_events();
		actions_pressed = 0;
		if (pause_requested)
		{
			pause_requested = false;
			console_lan_quit();
			return;
		}
		show_frame();
		wait_for_frame();
	}
}

/* ---- the pause ---- */

/* The pause's actions from the events since the last look: the arrows, Enter, Escape or Backspace and a game
   controller's Back; the players' own controls are players_pressed's */
static uint32_t pause_events(void)
{
	SDL_Event event;
	uint32_t pressed = 0;

	while (SDL_PollEvent(&event))
		switch (event.type)
		{
			case SDL_QUIT:
				quit();
				return 0;
			case SDL_KEYDOWN:
				if (event.key.repeat)
					break;
				switch (event.key.keysym.sym)
				{
					case SDLK_UP: pressed |= ACTION_BIT(ACTION_UP); break;
					case SDLK_DOWN: pressed |= ACTION_BIT(ACTION_DOWN); break;
					case SDLK_RETURN: case SDLK_KP_ENTER:
						if (event.key.keysym.mod & KMOD_ALT)
							toggle_full_screen();
						else
							pressed |= ACTION_BIT(ACTION_ACCEPT);
						break;
					case SDLK_ESCAPE: case SDLK_BACKSPACE: CASE_AC_BACK pressed |= PAUSE_BACK; break;
					case SDLK_F11: toggle_full_screen(); break;
					case SDLK_F3: next_pixels(); break;
					default: break;
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)
					pressed |= PAUSE_BACK;    /* (the button that pauses, as Escape) */
				break;
#ifdef SIMPSONS_TOUCH
			case SDL_FINGERDOWN:
			case SDL_FINGERMOTION:
			case SDL_FINGERUP:
				touch_event(&event);    /* (the touch screen's controls work the pause too: players_pressed) */
				break;
#endif
			case SDL_CONTROLLERDEVICEADDED:
				controller_added(event.cdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				controller_removed(event.cdevice.which);
				break;
			default:
				break;
		}
	return pressed;
}

/* The pause's actions from every player's controls newly pressed, as the menus take them: up and down, ATTACK the
   line chosen, JUMP or START go on. 'held': what each player held at the last look */
static uint32_t players_pressed(uint8_t held[PLAYERS])
{
	uint8_t players[PLAYERS], starts;
	uint32_t pressed = 0;

	read_players(players, &starts);
	for (int player = 0; player < PLAYERS; player++)
	{
		const uint8_t fresh = (uint8_t)(players[player] & ~held[player]);

		pressed |= (fresh & SIMPSONS_UP) ? ACTION_BIT(ACTION_UP) : 0u;
		pressed |= (fresh & SIMPSONS_DOWN) ? ACTION_BIT(ACTION_DOWN) : 0u;
		pressed |= (fresh & SIMPSONS_ATTACK) ? ACTION_BIT(ACTION_ACCEPT) : 0u;
		pressed |= (fresh & SIMPSONS_JUMP) ? PAUSE_BACK : 0u;
		held[player] = players[player];
	}
	return pressed | (starts ? PAUSE_BACK : 0u);
}

static void wait_for_frame(void);
static void fail(const char *message);

/* ---- The Simpsons' save state (game/state.h): the pause's SAVE STATE and LOAD STATE, one state in simpsons.state;
   a check's --state-save and --state-load at a frame's end, where the pause is ---- */

static char state_path[1100];         /* simpsons.state in the folder of what the game keeps, or --state's */
static uint64_t state_save_at;        /* --state-save FRAME: the state saved at that frame's end */
static bool state_load_pending;       /* --state-load: the state loaded at the end of the first frame it can be */

/* The port's part of a save state: the console's, the picture shown at its frame (the pause's after a load), and a
   script's place (a check's: the script goes on from the state's frame as it went on from it) */
static void port_state(simpsons_state *state)
{
	console_state_part(state);
	simpsons_state_part(state, "window");
	simpsons_state_bytes(state, &picture.left, sizeof(picture.left));
	simpsons_state_bytes(state, &picture.right, sizeof(picture.right));
	simpsons_state_bytes(state, picture_pixels, sizeof(picture_pixels));
	simpsons_script_state(script, state);
}

/* The game's state into state_path, or back from it (then the sound queued before it is dropped, and the pace starts
   again); what it came to, in the log too */
static simpsons_state_result save_or_load_state(bool loading)
{
	const uint64_t frame = simpsons_board.frame;
	const simpsons_state_result result = loading ? game_state_load(state_path, port_state)
		: game_state_save(state_path, port_state);

	if (result != SIMPSONS_STATE_DONE)
		fprintf(stderr, "State: %s %s at frame %" PRIu64 ": %s\n", loading ? "not loaded from" : "not saved in",
			state_path, frame, simpsons_state_problem(result));
	else if (!loading)
		fprintf(stderr, "State: saved in %s at frame %" PRIu64 "\n", state_path, frame);
	else
	{
		fprintf(stderr, "State: loaded from %s at frame %" PRIu64 ": the game at its frame %" PRIu64 "\n", state_path,
			frame, simpsons_board.frame);
		if (audio)
			SDL_ClearQueuedAudio(audio);
		next_frame = 0;
	}
	return result;
}

/* --state-save and --state-load, at the frame's end */
static void scripted_states(void)
{
	if (state_save_at && simpsons_board.frame == state_save_at)
	{
		state_save_at = 0;
		if (save_or_load_state(false) != SIMPSONS_STATE_DONE)
			fail("--state-save: the state cannot be saved at that frame.");
	}
	if (state_load_pending && game_state_possible())
	{
		state_load_pending = false;
		if (save_or_load_state(true) != SIMPSONS_STATE_DONE)
			fail("--state-load: the state cannot be loaded.");
	}
}

/* The game stopped, its sound too, the picture in grey under PAUSE: Escape, Backspace, a game controller's Back or a
   player's JUMP or START goes on, as CONTINUE does; SAVE STATE writes the game's state (save_or_load_state), LOAD
   STATE puts it back and shows its picture, the pause staying (its word on it under the lines); RESTART starts the
   same game again (console_restart_game), QUIT GAME ends it, back to the menu (console_quit_game). It keeps the
   frames' pace (wait_for_frame): with VSYNC a picture a frame behind it is not shown (frame_behind), and a pause that
   slept on its own showed its first picture only (tmnt-port's players' reports). What was held over it is no new press
   after it */
static void run_pause(void)
{
	static uint32_t frozen_pixels[SIMPSONS_VIDEO_MAX_WIDTH * SIMPSONS_VIDEO_HEIGHT];
	static uint32_t shown_pixels[SIMPSONS_VIDEO_MAX_WIDTH * SIMPSONS_VIDEO_HEIGHT];
	console_picture shown = { shown_pixels, SIMPSONS_VIDEO_MAX_WIDTH, picture.left, picture.right };    /* (WIDE's
		whole frame, as the game stopped on it) */
	int choice = CONSOLE_PAUSE_CONTINUE, dumps = 0;
	uint8_t players[PLAYERS], starts, held[PLAYERS];
	const char *message = NULL;

	pause_frame = simpsons_board.frame;
	pause_requested = false;
	if (audio)
		SDL_PauseAudioDevice(audio, 1);
	read_players(held, &starts);    /* (what was held as the pause came, the START that paused too: no new press) */
	memcpy(frozen_pixels, picture.pixels, sizeof(frozen_pixels));
	for (;;)
	{
		uint32_t pressed;
		bool dump;

		memcpy(shown_pixels, frozen_pixels, sizeof(shown_pixels));
		console_pause_overlay(&shown, choice);
		console_pause_message(&shown, message);
		upload_picture(&shown);
		present();    /* (the line chosen, a window resized or put full screen meanwhile) */
		pressed = pause_events() | players_pressed(held) | scripted_pause_action(&dump);
		if (dump)
		{
			char name[64];

			memcpy(picture_pixels, shown_pixels, sizeof(picture_pixels));
			snprintf(name, sizeof(name), "pause_%" PRIu64 "_%d", simpsons_board.frame, ++dumps);
			write_picture(name);
			memcpy(picture_pixels, frozen_pixels, sizeof(picture_pixels));
		}
		if (pressed & ACTION_BIT(ACTION_UP))
			choice = (choice + CONSOLE_PAUSE_ITEMS - 1) % CONSOLE_PAUSE_ITEMS;
		if (pressed & ACTION_BIT(ACTION_DOWN))
			choice = (choice + 1) % CONSOLE_PAUSE_ITEMS;
		if ((pressed & ACTION_BIT(ACTION_ACCEPT))
			&& (choice == CONSOLE_PAUSE_SAVE_STATE || choice == CONSOLE_PAUSE_LOAD_STATE))
		{
			const bool loading = choice == CONSOLE_PAUSE_LOAD_STATE;
			const simpsons_state_result result = save_or_load_state(loading);

			message = console_state_message(result, loading);
			if (loading && result == SIMPSONS_STATE_DONE)
			{
				memcpy(frozen_pixels, picture.pixels, sizeof(frozen_pixels));    /* (the state's picture) */
				shown.left = picture.left;
				shown.right = picture.right;
			}
		}
		else if (pressed & ACTION_BIT(ACTION_ACCEPT))
		{
			if (choice == CONSOLE_PAUSE_RESTART)
				console_restart_game();
			else if (choice == CONSOLE_PAUSE_QUIT)
				console_quit_game();
			break;
		}
		if (pressed & PAUSE_BACK)
			break;
		if (!fast)
			wait_for_frame();
	}
	read_players(players, &starts);
	console_controls_held(players);
	actions_pressed = 0;
	if (audio)
		SDL_PauseAudioDevice(audio, 0);
	next_frame = 0;    /* (the pace from here) */
}
#else
/* ---- The Simpsons Arcade Game's controls into the board's inputs, which it reads in the next frame (active low: a
   cleared bit is a pressed button): each player's as controls.ini binds them, the coins 5 to 8 and each game
   controller's Back, the test switch F2 ---- */

static bool button(SDL_GameController *pad, SDL_GameControllerButton which)
{
	return pad && SDL_GameControllerGetButton(pad, which) != 0;
}

static void read_controls(void)
{
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	uint8_t coins = 0xFF;
#ifndef SIMPSONS_HANDHELD
	static int quit_held;    /* the frames a game controller has held Back and Start together */
	bool quitting = false;
#endif

	for (int player = 0; player < PLAYERS; player++)
	{
		SDL_GameController *pad = controllers[player];
		uint8_t pressed = player_pressed(player);

#ifndef SIMPSONS_HANDHELD
		quitting = quitting || (button(pad, SDL_CONTROLLER_BUTTON_BACK) && button(pad, SDL_CONTROLLER_BUTTON_START));
#endif
#ifdef SIMPSONS_TOUCH
		if (player == 0)
			pressed |= touch_controls();    /* (the touch screen's: player 1's) */
#endif
		pressed |= controls_holds(player, pad, CONTROL_START) ? SIMPSONS_START : 0;
		simpsons_board.players[player] = (uint8_t)~pressed;
		if (keys[SDL_SCANCODE_5 + player] || button(pad, SDL_CONTROLLER_BUTTON_BACK))
			coins &= (uint8_t)~(SIMPSONS_COIN1 << player);
#ifdef SIMPSONS_TOUCH
		if (player == 0 && touch_start())
			coins &= (uint8_t)~SIMPSONS_COIN1;    /* (The Simpsons Arcade Game's touch screen: COIN) */
#endif
	}
	simpsons_board.coins = coins;
	if (keys[SDL_SCANCODE_F2] || service_held)
		simpsons_board.test &= (uint8_t)~SIMPSONS_TEST_SERVICE;
	else
		simpsons_board.test |= SIMPSONS_TEST_SERVICE;
#ifndef SIMPSONS_HANDHELD
	/* (Escape's quit for a game controller, which has no key for it: its Back and Start held together two seconds; a
	   controller in Big Picture has no keyboard in reach. A device leaves the game its own way: HOME, MENU) */
	quit_held = quitting ? quit_held + 1 : 0;
	if (quit_held >= QUIT_HOLD_FRAMES)
		quit();
#endif
}

/* ---- The Simpsons Arcade Game's EEPROM, kept: simpsons.nv in the folder of what the game keeps, read as the game starts,
   written when the game has changed it (the board says so: simpsons_eeprom_written), at the end of that frame ---- */

static char eeprom_path[1100];
static const char *eeprom_option;     /* --eeprom FILE: the EEPROM kept there, a scripted run's too (a check's) */
static bool eeprom_changed;

static void eeprom_written(void)
{
	eeprom_changed = true;
}

static void load_eeprom(void)
{
	uint8_t data[SIMPSONS_EEPROM_SIZE];
	FILE *input = fopen(eeprom_path, "rb");
	size_t got;

	if (!input)
	{
		fprintf(stderr, "EEPROM: the factory's (no %s)\n", eeprom_path);
		return;
	}
	got = fread(data, 1, sizeof(data), input);
	if (got == sizeof(data) && fgetc(input) == EOF)
	{
		memcpy(simpsons_board.eeprom.data, data, sizeof(data));
		fprintf(stderr, "EEPROM: %s\n", eeprom_path);
	}
	else
		fprintf(stderr, "EEPROM: %s is not 128 bytes, the factory's taken\n", eeprom_path);
	fclose(input);
}

static void save_eeprom(void)
{
	FILE *output;

	if (!eeprom_changed || !eeprom_path[0])
		return;
	eeprom_changed = false;
	if ((output = fopen(eeprom_path, "wb")) == NULL)
	{
		fprintf(stderr, "EEPROM: %s cannot be written\n", eeprom_path);
		return;
	}
	fwrite(simpsons_board.eeprom.data, 1, SIMPSONS_EEPROM_SIZE, output);
	fclose(output);
}
#endif

/* ---- The Simpsons' settings (OPTIONS), kept in simpsons.ini ---- */

#ifdef SIMPSONS_CONSOLE
static char settings_path[1100];

static void save_settings(void)
{
	const console_settings_t *settings = &console_settings;
	char continues[16] = "unlimited";    /* (ARCADE's continues: unlimited or 0-9) */
	FILE *output;

	if (scripted || !settings_path[0] || console_lan_playing() || (output = fopen(settings_path, "w")) == NULL)
		return;    /* (a LAN game's: the host's settings, not this device's) */
	if (settings->game_continues != CONSOLE_CONTINUES_UNLIMITED)
		snprintf(continues, sizeof(continues), "%d", settings->game_continues);
	if (!settings->full_screen)
		launcher_full_screen = false;    /* (a window chosen in OPTIONS over Big Picture's full screen: the player's) */
	/* (each key with what it takes, in a comment before it: the Recomp Standards' readable settings; the game reads the
	   keys alone, load_settings) */
	fprintf(output, "; " TITLE "'s settings, OPTIONS' (the game writes this file again when they change)\n"
		"; window_scale: the window, in times the game's picture, 1 to 6\nwindow_scale=%d\n"
		"; full_screen: 1 the whole screen, 0 a window (F11 or Alt+Enter in the game)\nfull_screen=%d\n"
		"; pixels: sharp, smooth, bilinear or crt (F3 in the game)\npixels=%s\n"
		"; integer_scale: 1 the picture a whole number of times its pixels, 0 as large as it fits\n"
		"integer_scale=%d\n"
		"; wide: off (the cabinet's picture), 16:10, 16:9, 20:9, or auto (the screen's shape)\nwide=%s\n"
		"; vsync: 1 each frame shown at the screen's refresh, 0 not\nvsync=%d\n"
		"; volume: 0 to 10, 5 the game's own\nvolume=%d\n"
		"; fps: 1 the frames a second at the picture's top right, 0 not\nfps=%d\n"
		"; difficulty: easy, normal, difficult or very difficult (the cabinet's)\ndifficulty=%s\n"
		"; lives: the lives a player starts ARCADE with, 1 to 7\nlives=%d\n"
		"; continues: ARCADE's continues, 0 to 9 or unlimited\ncontinues=%s\n"
		"; demo_sound: 1 the sound while the game shows itself, 0 none (the cabinet's)\ndemo_sound=%d\n"
		"; screen: normal or upside down (the cabinet's)\nscreen=%s\n"
		"; sound: mono (as the cabinet is wired) or stereo\nsound=%s\n"
		"; bart_shirt: orange (Bart's shirt orange, as on the show, Barney's salmon) or arcade (the cabinet's)\n"
		"bart_shirt=%s\n",
		settings->window_scale, settings->full_screen && !launcher_full_screen, console_pixel_names[settings->pixels],
		settings->integer_scale,
		console_wide_names[settings->wide], settings->vsync, settings->volume, settings->fps,
		console_difficulty_names[settings->game_difficulty], settings->game_lives, continues, settings->demo_sound,
		settings->upside_down ? "upside down" : "normal", settings->stereo ? "stereo" : "mono",
		settings->bart_shirt ? "orange" : "arcade");
	if (settings->name[0])
		fprintf(output, "; name: this device's in MULTIPLAYER's rooms\nname=%s\n", settings->name);
	fclose(output);
}

static void load_settings(void)
{
	console_settings_t *settings = &console_settings;
	char line[128], text[32];
	int number;
	FILE *input = fopen(settings_path, "r");

	if (!input)
		return;
	while (fgets(line, sizeof(line), input))
	{
		char *end = line + strlen(line);

		while (end > line && (end[-1] == '\n' || end[-1] == '\r' || end[-1] == ' '))
			*--end = '\0';
		{
			/* (a key and its value as "key = value" too, as an editor of INI files may write a key the file did not
			   have: RecompLauncher's, docs/package/recomplauncher.json) */
			char *equals = strchr(line, '='), *key_end = equals, *value = equals ? equals + 1 : NULL;

			while (key_end && key_end > line && (key_end[-1] == ' ' || key_end[-1] == '\t'))
				key_end--;
			while (value && (*value == ' ' || *value == '\t'))
				value++;
			if (equals && (key_end != equals || value != equals + 1))
			{
				*key_end = '=';
				memmove(key_end + 1, value, strlen(value) + 1);
			}
		}
		if (sscanf(line, "window_scale=%d", &number) == 1 && number >= 1 && number <= 6)
			settings->window_scale = number;
		else if (sscanf(line, "full_screen=%d", &number) == 1)
			settings->full_screen = number != 0;
		else if (sscanf(line, "volume=%d", &number) == 1 && number >= 0 && number <= 10)
			settings->volume = number;
		else if (sscanf(line, "fps=%d", &number) == 1)
			settings->fps = number != 0;
		else if (sscanf(line, "vsync=%d", &number) == 1)
			settings->vsync = number != 0;
		else if (sscanf(line, "integer_scale=%d", &number) == 1)
			settings->integer_scale = number != 0;
		else if (sscanf(line, "lives=%d", &number) == 1 && number >= CONSOLE_LIVES_FEWEST && number <= CONSOLE_LIVES_MOST)
			settings->game_lives = number;
		else if (strcmp(line, "continues=unlimited") == 0)
			settings->game_continues = CONSOLE_CONTINUES_UNLIMITED;    /* (ARCADE's continues: unlimited or 0-9) */
		else if (sscanf(line, "continues=%d", &number) == 1 && number >= 0 && number <= CONSOLE_CONTINUES_MOST)
			settings->game_continues = number;
		else if (sscanf(line, "demo_sound=%d", &number) == 1)
			settings->demo_sound = number != 0;
		else if (strncmp(line, "screen=", 7) == 0)
			settings->upside_down = strcmp(line + 7, "upside down") == 0;
		else if (strncmp(line, "sound=", 6) == 0)
			settings->stereo = strcmp(line + 6, "stereo") == 0;    /* (OPTIONS > SOUND: stereo or mono) */
		else if (strncmp(line, "name=", 5) == 0 && line[5] && strlen(line + 5) <= CONSOLE_NAME_LENGTH
			&& strspn(line + 5, " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-") == strlen(line + 5))
			snprintf(settings->name, sizeof(settings->name), "%s", line + 5);    /* (MULTIPLAYER > NAME) */
		else if (strncmp(line, "bart_shirt=", 11) == 0)
			settings->bart_shirt = strcmp(line + 11, "arcade") != 0;    /* (Bart's shirt: orange, or the arcade's) */
		else if (strncmp(line, "difficulty=", 11) == 0)
		{
			for (int kind = 0; kind < CONSOLE_DIFFICULTIES; kind++)
				if (strcmp(line + 11, console_difficulty_names[kind]) == 0)
					settings->game_difficulty = kind;
		}
		else if (sscanf(line, "pixels=%31s", text) == 1)
		{
			for (int kind = 0; kind < PIXELS_KINDS; kind++)
				if (strcmp(text, console_pixel_names[kind]) == 0)
					settings->pixels = kind;
		}
		else if (sscanf(line, "wide=%31s", text) == 1)
		{
			for (int kind = 0; kind < WIDE_KINDS; kind++)
				if (strcmp(text, console_wide_names[kind]) == 0)
					settings->wide = kind;    /* (by name: the enum only grows at its end) */
		}
	}
	fclose(input);
}
#endif

/* VSYNC (The Simpsons' OPTIONS > VIDEO, The Simpsons Arcade Game's always): the renderer shows each frame at the screen's
   refresh, unless the frames run as fast as they can (--fast). The game keeps its own pace, the board's 59.19 frames
   a second (wait_for_frame): a faster screen (120 Hz) shows a frame twice. A device's screen keeps the system's own
   refresh (and the Anbernic's SDL 2.0.12 has no SDL_RenderSetVSync), as tmnt-port's */
static void apply_vsync(void)
{
#ifndef SIMPSONS_HANDHELD
#ifdef SIMPSONS_CONSOLE
	const bool wanted = console_settings.vsync && !fast && !no_vsync;
#else
	const bool wanted = !fast && !no_vsync;
#endif

	if (renderer && wanted != vsync_on)
		vsync_on = SDL_RenderSetVSync(renderer, wanted ? 1 : 0) == 0 ? wanted : false;
#endif
}

#ifdef SIMPSONS_CONSOLE
/* The window as the settings say: full screen or not, and out of it the picture (WIDE's width) times window_scale;
   done again only when they change (OPTIONS > VIDEO, F11), so a window sized by hand keeps its size otherwise */
static void apply_settings(void)
{
	static int applied_scale = -1, applied_full = -1, applied_wide = -1;

	game_set_stereo(console_settings.stereo);    /* (OPTIONS > SOUND) */
	apply_vsync();
#ifdef SIMPSONS_TOUCH_CHECK
	if (check_screen_width > 0)
		return;    /* (the check's phone or tablet screen keeps its size) */
#endif
	if (applied_scale == console_settings.window_scale && applied_full == (int)console_settings.full_screen
		&& applied_wide == console_wide_in_effect())
		return;
	applied_scale = console_settings.window_scale;
	applied_full = (int)console_settings.full_screen;
	applied_wide = console_wide_in_effect();    /* (WIDE AUTO's as the screen's shape asks: the window for it) */
#ifdef __SWITCH__
	return;    /* (the window is the Switch's 1280 x 720 screen already. Full screen would make it SDL's desktop mode,
	              which devkitPro's SDL has 1920 x 1080 always (SWITCH_VideoInit): the picture drawn for 1080p and
	              shown in part on the 720p screen, the players' issue 2. Docked, the system scales the 720p up) */
#endif
	SDL_SetWindowFullscreen(window, console_settings.full_screen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	if (!console_settings.full_screen)
		SDL_SetWindowSize(window, view_width() * console_settings.window_scale,
			SIMPSONS_VIDEO_HEIGHT * console_settings.window_scale);
#ifndef SIMPSONS_HANDHELD
	{
		int width = 0, height = 0;    /* (the log: what the settings made of the window, full screen or its size) */

		SDL_GetWindowSize(window, &width, &height);
		fprintf(stderr, "%s %dx%d%s\n", console_settings.full_screen ? "Full screen" : "Window", width, height,
			launcher_full_screen ? " (RecompLauncher's Big Picture)" : "");
	}
#endif
}
#endif

/* ---- the frames ---- */

/* The frame's time: the board's 59.185 frames a second from the last one, slept for but its last millisecond (a
   frame shown with VSYNC has waited for the screen's refresh already, frame_behind).
   A frame that ends behind its time leaves the next one undrawn (late_skip: on_frame_done, game_set_draw; the board's
   state is the same either way), LATE_SKIPS_MOST in a row at most: a device that takes longer than a frame's time to
   draw one (the Switch, the players' issue 21: 37 to 50 pictures a second, the game and its sound as slow and the
   sound breaking up) keeps the game's pace and its sound, with fewer pictures; a frame not drawn costs a third of
   one drawn. Behind by half a frame's time, or by a whole one with VSYNC, whose wait for the screen's refresh is in
   it (that frame would not be shown anyway, frame_behind). Where frames go undrawn one soon after another, within
   LATE_PACE_NEAR frames, the machine is slow: for LATE_PACE_FRAMES the frames drawn are all shown (late_pace); one
   alone, a busy moment's or a slower screen's every second or so, changes nothing else */
#define LATE_SKIPS_MOST     3
#define LATE_PACE_NEAR      30
#define LATE_PACE_FRAMES    120

static void wait_for_frame(void)
{
	const uint64_t frequency = SDL_GetPerformanceFrequency();
	const uint64_t period = frequency * FRAME_RATE_DENOMINATOR / FRAME_RATE_NUMERATOR;
	uint64_t now = SDL_GetPerformanceCounter();

	if (next_frame == 0 || now > next_frame + 4 * period)
		next_frame = now;    /* (behind by more than a few frames: the pace starts again from here) */
	next_frame += period;
	late_skip = now > next_frame + (vsync_on ? period : period / 2) && late_skips < LATE_SKIPS_MOST;
	late_skips = late_skip ? late_skips + 1 : 0;
	if (late_skip)
	{
		if (late_since < LATE_PACE_NEAR)
			late_pace = LATE_PACE_FRAMES;
		late_since = 0;
	}
	else
	{
		late_since += late_since < LATE_PACE_FRAMES ? 1 : 0;
		late_pace -= late_pace > 0 ? 1 : 0;
	}
	while ((now = SDL_GetPerformanceCounter()) < next_frame)
	{
		const uint64_t left_ms = (next_frame - now) * 1000 / frequency;

		if (left_ms > 1)
			SDL_Delay((Uint32)(left_ms - 1));
	}
}

/* --video-dump: the window's picture as a 24-bit BMP into dump_dir, NAME.bmp, as kref writes them; in The Simpsons
   where the console stands, on the standard output */
/* A 32-bit value at 'at', little endian (a .bmp's header) */
static void put_le32(uint8_t *at, uint32_t value)
{
	at[0] = (uint8_t)value;
	at[1] = (uint8_t)(value >> 8);
	at[2] = (uint8_t)(value >> 16);
	at[3] = (uint8_t)(value >> 24);
}

static void write_picture(const char *name)
{
#ifdef SIMPSONS_CONSOLE
	const int width = SIMPSONS_VIDEO_WIDTH + picture.left + picture.right, stride = picture.stride;    /* (WIDE's) */
#else
	const int width = SIMPSONS_VIDEO_WIDTH, stride = SIMPSONS_VIDEO_WIDTH;
#endif
	const int height = SIMPSONS_VIDEO_HEIGHT;
	const uint32_t row_bytes = ((uint32_t)width * 3u + 3u) & ~3u, size = 54u + row_bytes * (uint32_t)height;
	uint8_t header[54] = { 'B', 'M' };
	char path[1200];
	FILE *output;

	snprintf(path, sizeof(path), "%s/%s.bmp", dump_dir, name);
	if ((output = fopen(path, "wb")) == NULL)
	{
		fprintf(stderr, "%s: cannot be written\n", path);
		return;
	}
	/* (each field by itself: MSVC 19.50's /O2 left the image's size, bytes 34-37, 0 when one loop wrote the four) */
	put_le32(header + 2, size);
	put_le32(header + 18, (uint32_t)width);
	put_le32(header + 22, (uint32_t)height);
	put_le32(header + 34, row_bytes * (uint32_t)height);
	header[10] = 54;
	header[14] = 40;
	header[26] = 1;
	header[28] = 24;
	fwrite(header, 1, sizeof(header), output);
	for (int y = height - 1; y >= 0; y--)
	{
		static const uint8_t pad[3] = { 0, 0, 0 };

		for (int x = 0; x < width; x++)
		{
			const uint32_t pixel = picture_pixels[y * stride + x];
			const uint8_t bgr[3] = { (uint8_t)pixel, (uint8_t)(pixel >> 8), (uint8_t)(pixel >> 16) };

			fwrite(bgr, 1, 3, output);
		}
		fwrite(pad, 1, row_bytes - (uint32_t)width * 3u, output);
	}
	fclose(output);
#ifdef SIMPSONS_CONSOLE
	{
		char text[1024];

		console_describe(text, sizeof(text));
		printf("%s: %s\n", name, text);
		fflush(stdout);
	}
#endif
}

static void dump_frame(uint64_t number)
{
	for (int index = 0; index < dump_count; index++)
		if (dump_frames[index] == number)
		{
			char name[32];

			snprintf(name, sizeof(name), "frame_%" PRIu64, number);
			write_picture(name);
		}
}

/* Whether frame 'number' is one --video-dump writes */
static bool dumped(uint64_t number)
{
	for (int index = 0; index < dump_count; index++)
		if (dump_frames[index] == number)
			return true;
	return false;
}

/* --slow: the frame drawn takes its milliseconds more, and every SLOW_REPORT_FRAMES frames how the pace held */
#define SLOW_REPORT_FRAMES  600

static void slow_frame(bool drawn, uint64_t number)
{
	uint64_t now;

	if (drawn)
	{
		SDL_Delay((Uint32)slow_ms);
		slow.drawn++;
	}
	if (slow.since == 0)
		slow.since = SDL_GetPerformanceCounter();
	if (++slow.frames < SLOW_REPORT_FRAMES)
		return;
	now = SDL_GetPerformanceCounter();
	fprintf(stderr, "Slow %d ms, to frame %" PRIu64 ": %.2f frames a second (the board's %.2f), %u of %u drawn, %u shown,"
		" the sound out %u times\n", slow_ms, number,
		slow.frames * (double)SDL_GetPerformanceFrequency() / (double)(now - slow.since),
		(double)FRAME_RATE_NUMERATOR / FRAME_RATE_DENOMINATOR, slow.drawn, slow.frames, slow.shown, slow.sound_empty);
	slow.since = now;
	slow.frames = slow.drawn = slow.shown = slow.sound_empty = 0;
}

#define HURRIED_SHOWN       16    /* (the console's hurried frames: one drawn and shown in these, the window kept
                                     alive) */
static bool frame_drawn = true;    /* (whether the frame done was drawn: game_set_draw) */

static void on_frame_done(void)
{
	const uint64_t number = frames_before + simpsons_board.frame;
#ifdef SIMPSONS_CONSOLE
	const bool hurried = console_hurried();    /* (the Konami game's black before its stage: as fast as it can) */
#else
	const bool hurried = false;
#endif

	if (slow_ms > 0)
		slow_frame(frame_drawn, number);
	read_events();
#ifdef SIMPSONS_CONSOLE
	if (lan_given && lan_hashes)
	{
		fprintf(lan_hashes, "%" PRIu32 " %016" PRIx64 "\n", console_lan_frame_number(), simpsons_state_hash());
		fflush(lan_hashes);
	}
	lan_given = false;
	if (script)
		simpsons_script_apply(script, simpsons_board.frame);
	if (console_lan_playing())
	{
		lan_controls();    /* (MULTIPLAYER's game: every device's controls, from the network) */
		lan_given = console_lan_playing();
	}
	else
		read_controls();    /* (a script's controls are the players': the console puts them on their slots) */
	console_wide_frame();    /* (WIDE: from the next frame drawn) */
	console_scores_frame();    /* (the high scores kept: into the game after the boot, a new one written) */
	if (wide_check)
	{
		char text[512];

		if (console_wide_check(text, sizeof(text)))
			printf("frame %" PRIu64 ": WIDE's sides not their map's: %s\n", number, text);
		if (console_wide_spawn_check(text, sizeof(text)))
			printf("frame %" PRIu64 ": put in in WIDE's sides' sight: %s\n", number, text);
	}
#else
	if (script)
		simpsons_script_apply(script, simpsons_board.frame);
	else
		read_controls();
#endif
	game_end_frame();
	queue_sound();
	if (hashes)
		fprintf(hashes, "%" PRIu64 " %016" PRIx64 "\n", number, simpsons_state_hash());
	if (video_hashes)
		fprintf(video_hashes, "%" PRIu64 " %016" PRIx64 "\n", number, simpsons_video_hash(game_frame()));
	if (sound_hashes)
		fprintf(sound_hashes, "%" PRIu64 " %016" PRIx64 " %016" PRIx64 "\n", number, game_sound_samples_hash(),
			game_sound_state_hash());
	if (hurried)
		next_frame = SDL_GetPerformanceCounter();    /* (no pace to be behind: with VSYNC a hurried picture, BOSS RUSH's
		                                                card, was left out as late, frame_behind, all but the first if
		                                                that: the players' issue 18; the pace goes on from the last) */
	if (frame_drawn && (!hurried || number % HURRIED_SHOWN == 0 || dumped(number)))
		show_frame();
	dump_frame(number);
#ifdef SIMPSONS_CONSOLE
	if (pause_requested && console_lan_playing())
	{
		console_lan_quit();    /* (a LAN game has no pause: Escape, Back or the background leave it) */
		lan_given = false;    /* (the next frame this device's alone: the game ended at it) */
	}
	else if (pause_requested && console_can_pause())
		run_pause();
	pause_requested = false;
	scripted_states();    /* (--state-save, --state-load: where the pause's are) */
	if (console_settings.changed)
	{
		console_settings.changed = false;
		apply_settings();
		save_settings();
	}
	if (console_quit_requested())
		quit();
	if (console_lan_reset_wanted())
	{
		frames_before += simpsons_board.frame;    /* (a LAN game's start: the program left, the board reset: main's) */
		longjmp(lan_restart_point, 1);
	}
#else
	save_eeprom();
#endif
	late_skip = false;
	if (script && script_frames && number >= script_frames)
		quit();
	else if (!fast && !hurried)
		wait_for_frame();
	/* (the next frame drawn but in a hurry: then one in HURRIED_SHOWN; or behind the pace, late_skip: not then; those
	   dumped and, for --video-hashes, every one always) */
	frame_drawn = (!hurried || (number + 1) % HURRIED_SHOWN == 0) && !late_skip;
	game_set_draw(frame_drawn || dumped(number + 1) || video_hashes);
	frame_drawn = frame_drawn || dumped(number + 1);
}

/* ---- the start ---- */

#ifdef __WINRT__
/* The package's LocalState folder with its separator (SDL's path of it, in UTF-8: the C library's files take it, a
   UWP app's own folder): the ROM set, the pack, what the game keeps, its log. The caller frees it (SDL_free) */
static char *uwp_local_folder(void)
{
	const char *local = SDL_WinRTGetFSPathUTF8(SDL_WINRT_PATH_LOCAL_FOLDER);
	const size_t length = local ? strlen(local) + 2 : 1;
	char *folder = SDL_malloc(length);

	if (folder)
		snprintf(folder, length, "%s%s", local ? local : "", local && local[0] ? "\\" : "");
	return folder;
}
#endif

/* Where the game keeps what it writes (its settings, its controls, its EEPROM, its log): beside the executable on
   Windows; elsewhere the user's own folder for the game (SDL_GetPrefPath: ~/.local/share/The Simpsons,
   ~/Library/Application Support/The Simpsons), as the executable's may not be written, unless a portable.txt beside
   the game (beside the .app on macOS) asks for its folder, as on Windows; on a device its own folder (device_folder:
   the Switch's on the SD card, the handheld's beside the executables on its card). The caller frees it (SDL_free) */
static char *saves_folder(void)
{
#ifdef __SWITCH__
	return SDL_strdup(SWITCH_FOLDER "/");
#elif defined(__ANDROID__)
	return SDL_GetPrefPath("", SAVES_NAME);    /* (the app's own folder, its internal storage: tmnt-port's) */
#elif defined(__APPLE__) && TARGET_OS_IOS
	return ios_documents();    /* (the app's Documents, the Files app's: the player can see and keep them) */
#elif defined(__WINRT__)
	return uwp_local_folder();    /* (the package's own LocalState: its installed folder is read-only) */
#elif defined(_WIN32) || defined(SIMPSONS_HANDHELD)
	return SDL_GetBasePath();
#else
	char *base = SDL_GetBasePath(), *folder = NULL;
	char beside_app[1100], probe[1200];
	const char *place = base && app_folder(base, beside_app, sizeof(beside_app)) ? beside_app : base;
	FILE *portable;

	if (place)
	{
		snprintf(probe, sizeof(probe), "%s" PORTABLE_FILE, place);
		if ((portable = fopen(probe, "rb")) != NULL)
		{
			fclose(portable);
			folder = SDL_strdup(place);
		}
	}
	SDL_free(base);
	return folder ? folder : SDL_GetPrefPath("", SAVES_NAME);
#endif
}

/* The log into FILE_NAME.log in 'folder' (with its separator), the last runs' moved before it to FILE_NAME.1.log and
   FILE_NAME.2.log, the older ones dropped (LOG_RUNS: the last runs' logs in a known place, the Recomp Standards'; a
   launcher sends the newest with a crash report) */
static void log_into(const char *folder)
{
	char path[1100], older[1100];

	for (int run = LOG_RUNS - 1; run > 0; run--)
	{
		snprintf(older, sizeof(older), "%s" FILE_NAME ".%d.log", folder, run);
		if (run > 1)
			snprintf(path, sizeof(path), "%s" FILE_NAME ".%d.log", folder, run - 1);
		else
			snprintf(path, sizeof(path), "%s" FILE_NAME ".log", folder);
		remove(older);
		rename(path, older);
	}
	snprintf(path, sizeof(path), "%s" FILE_NAME ".log", folder);
	if (freopen(path, "w", stderr))
		setvbuf(stderr, NULL, _IONBF, 0);
}

/* The log: on Windows a game started with nowhere for its standard error to go (from Explorer, not a terminal), and on
   Linux and macOS one whose standard error is not a terminal (a launcher's, Steam's, the Finder's), writes it to
   FILE_NAME.log in the folder of what the game keeps; on the Switch, which has no console (and SDL no message boxes
   there), always, in its folder on the SD card; on Android always, in the app's folder on the storage; elsewhere it
   goes where it is sent (the terminal, the handheld's launcher's file). Each keeps the last runs' too (log_into) */
static void open_log(void)
{
#ifdef __SWITCH__
	mkdir("sdmc:/switch", 0777);
	mkdir(SWITCH_FOLDER, 0777);
	log_into(SWITCH_FOLDER "/");
#endif
#ifdef __ANDROID__
	/* (Android's standard error goes nowhere: the log in the app's folder on the storage, Android/data/<the
	   app>/files, where the player puts the ROM set and can read it) */
	if (SDL_AndroidGetExternalStoragePath())
	{
		char folder[1100];

		snprintf(folder, sizeof(folder), "%s/", SDL_AndroidGetExternalStoragePath());
		log_into(folder);
	}
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
	{
		/* (an app's standard error reaches no one: the log in its Documents, where the player puts the ROM set and the
		   Files app shows it) */
		char *documents = ios_documents();

		log_into(documents ? documents : "");
		SDL_free(documents);
	}
#endif
#ifdef __WINRT__
	{
		/* (a UWP app's standard error reaches no one: the log in its LocalState, beside the ROM set) */
		char *local = uwp_local_folder();

		log_into(local ? local : "");
		SDL_free(local);
	}
#elif defined(_WIN32)
	if (_fileno(stderr) < 0)
	{
		char *saves = saves_folder();

		log_into(saves ? saves : "");
		SDL_free(saves);
	}
#endif
#if !defined(_WIN32) && !defined(SIMPSONS_HANDHELD) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS)
	if (!isatty(fileno(stderr)))
	{
		char *saves = saves_folder();

		log_into(saves ? saves : "");
		SDL_free(saves);
	}
#endif
}

static bool exists(const char *path)
{
	FILE *probe = fopen(path, "rb");

	if (!probe)
		return false;
	fclose(probe);
	return true;
}

#ifdef SIMPSONS_HANDHELD
#define ROM_SET_FILE    "simpsons.zip"    /* (first_run.c's: the ROM set, or its folder rom with its files or .zip) */
#define ROM_SET_FOLDER  "rom"
#define ROM_SET_PROBE   "072-g02.16c"     /* a file of the set by MAME's name: a folder of its files */

#ifdef __SWITCH__
/* A file of 1.0.0's folder copied into SWITCH_FOLDER, when that has none of the name and, with a 'mark', the file's
   first bytes have it (the file of that name is these games'); whether it was copied */
static bool switch_copy_old(const char *name, const char *mark)
{
	char from[300], to[300], bytes[4097];
	FILE *input, *output;
	size_t count;
	bool done = true;

	snprintf(from, sizeof(from), SWITCH_OLD_FOLDER "/%s", name);
	snprintf(to, sizeof(to), SWITCH_FOLDER "/%s", name);
	if (exists(to) || (input = fopen(from, "rb")) == NULL)
		return false;
	if (mark)
	{
		count = fread(bytes, 1, sizeof(bytes) - 1, input);
		bytes[count] = '\0';
		if (!strstr(bytes, mark))
		{
			fclose(input);
			return false;
		}
		rewind(input);
	}
	if ((output = fopen(to, "wb")) == NULL)
	{
		fclose(input);
		return false;
	}
	while (done && (count = fread(bytes, 1, sizeof(bytes), input)) > 0)
		done = fwrite(bytes, 1, count, output) == count;
	done = !ferror(input) && done;
	fclose(input);
	done = fclose(output) == 0 && done;
	if (!done)
		remove(to);
	return done;
}

/* 1.0.0 kept both games' files in sdmc:/switch/simpsons/, which The Simpsons: Hit & Run's port uses too (the players'
   issue 2). At the first start with no SWITCH_FOLDER, these games' own files there are copied into it (both games',
   whichever starts first), the old folder left as it was: what else is in it is the other game's. The settings and
   the controls are taken only when they are these games' (their own key, their own first line); the others by their
   names, these games' alone. A ROM set as the folder rom is not taken (its message says where it goes now). The names
   copied, into 'copied' for the log; how many */
static int switch_take_old_folder(char *copied, size_t size)
{
	static const struct
	{
		const char *name, *mark;
	} files[] = {
		{ ROM_SET_FILE, NULL },
		{ GAME_ASSETS_PACK_FILE, NULL },
		{ "simpsons.ini", "bart_shirt=" },             /* (The Simpsons' SETTINGS_FILE) */
		{ "simpsons.scores", NULL },                   /* (SCORES_FILE) */
		{ "simpsons.state", NULL },                    /* (STATE_FILE) */
		{ "simpsons.nv", NULL },                       /* (The Simpsons Arcade Game's EEPROM_FILE) */
		{ CONTROLS_FILE, "; The Simpsons' controls" }, /* (controls.c's first line) */
	};
	struct stat folder;
	int count = 0;

	copied[0] = '\0';
	if (stat(SWITCH_FOLDER, &folder) == 0 || stat(SWITCH_OLD_FOLDER, &folder) != 0)
		return 0;
	mkdir("sdmc:/switch", 0777);
	mkdir(SWITCH_FOLDER, 0777);
	for (size_t index = 0; index < sizeof(files) / sizeof(files[0]); index++)
		if (switch_copy_old(files[index].name, files[index].mark))
		{
			const size_t length = strlen(copied);

			snprintf(copied + length, size - length, "%s%s", count ? ", " : "", files[index].name);
			count++;
		}
	return count;
}
#endif

static void fail(const char *message);

/* A device's own folder, where the player puts the ROM set (or a pack) and the game makes its assets pack of it: the
   Switch's sdmc:/switch/simpsons-arcade/, the handheld's PORTS/simpsons/ (beside both executables), both games'; Android's
   app folder on its storage (Android/data/<the app>/files/, where its log is), each game's own. The caller frees it
   (SDL_free) */
static char *device_folder(void)
{
#ifdef __SWITCH__
	return SDL_strdup(SWITCH_FOLDER "/");
#elif defined(__ANDROID__)
	const char *storage = SDL_AndroidGetExternalStoragePath();
	const size_t length = storage ? strlen(storage) + 2 : 1;
	char *folder = SDL_malloc(length);

	if (folder)
		snprintf(folder, length, "%s%s", storage ? storage : "", storage ? "/" : "");
	return folder;
#elif defined(__APPLE__) && TARGET_OS_IOS
	return ios_documents();    /* (the app's Documents: the Files app's, each game's own) */
#elif defined(__WINRT__)
	return uwp_local_folder();    /* (the package's LocalState: Device Portal's File explorer reaches it) */
#else
	return SDL_GetBasePath();
#endif
}

/* The ROM set in the folder 'place' into 'source': simpsons.zip there, or its folder rom with the set's .zip or its
   files; "" when there is none */
static void device_rom_set(const char *place, char *source, size_t size)
{
	char probe[1200];

	snprintf(source, size, "%s" ROM_SET_FILE, place);
	if (exists(source))
		return;
	snprintf(source, size, "%s" ROM_SET_FOLDER, place);
	snprintf(probe, sizeof(probe), "%s/" ROM_SET_FILE, source);
	if (exists(probe))
	{
		const size_t length = strlen(source);

		snprintf(source + length, size - length, "/" ROM_SET_FILE);    /* (the .zip in the folder rom) */
		return;
	}
	snprintf(probe, sizeof(probe), "%s/" ROM_SET_PROBE, source);
	if (!exists(probe))
		source[0] = '\0';
}

/* The assets in the device's folder, for the releases' packages, which carry none (tmnt-port's device_assets): the
   pack simpsons.assets there; else, or when it is another version's, the pack made there now from the ROM set there,
   as a computer's first run makes it (first_run.c's pack_made), with no window; then loaded. false, and what is wrong
   in 'problem', when there is none */
static bool device_assets(char *found, size_t size, char *problem, size_t problem_size)
{
	char *folder = device_folder();
	const char *place = folder ? folder : "";
	char source[1100], stale[400] = "";
	const char *failed;
	bool ready = false;

	snprintf(found, size, "%s" GAME_ASSETS_PACK_FILE, place);
	if (exists(found))
	{
		if ((failed = game_load(found)) == NULL)
			ready = true;
		else
		{
			/* (another version's, or not whole: made again from the ROM set) */
			fprintf(stderr, "%s: %s\n", found, failed);
			snprintf(stale, sizeof(stale), GAME_ASSETS_PACK_FILE ": %s. ", failed);
		}
	}
	if (!ready)
	{
		device_rom_set(place, source, sizeof(source));
		if (!source[0])
#if defined(__APPLE__) && TARGET_OS_IOS
			/* (the app's Documents as the player finds it: in the Files app, or from a computer) */
			snprintf(problem, problem_size, "%sNo ROM set: put " ROM_SET_FILE " (MAME's simpsons set, 4 Players World) "
				"in this game's folder, in the Files app: On My iPhone (On My iPad) > " TITLE ". From a computer: Finder "
				"(or the Apple Devices app on Windows), the device, Files, " TITLE ". Then start the game again: the "
				"first start makes the game's assets of it (a few seconds).", stale);
#elif defined(__WINRT__)
			/* (the folder as the player reaches it on an Xbox: Device Portal, from a computer's browser) */
			snprintf(problem, problem_size, "%sNo ROM set: put " ROM_SET_FILE " (MAME's simpsons set, 4 Players World) "
				"in this app's LocalState folder: on the Xbox, Device Portal (its address in Dev Home) > File explorer "
				"> LocalAppData > this game's package > LocalState > Upload. Then start the game again: the first start "
				"makes the game's assets of it (a few seconds). The folder: %s", stale, place);
#else
			snprintf(problem, problem_size, "%sNo ROM set: put " ROM_SET_FILE " (MAME's simpsons set, 4 Players World) "
				"in %s and start the game again. The first start makes the game's assets of it (a few seconds).", stale,
				place);
#endif
		else
		{
			fprintf(stderr, "The assets out of the ROM set %s into %s\n", source, found);
			if ((failed = game_load_rom_set(source)) != NULL || (failed = game_assets_pack_write(found)) != NULL
				|| (failed = game_load(found)) != NULL)
				snprintf(problem, problem_size, "The assets cannot be made of %s: %s", source, failed);
			else
			{
				fprintf(stderr, "The assets pack written (version %08X)\n", (unsigned)game_assets_version());
				ready = true;
			}
		}
	}
	SDL_free(folder);
	return ready;
}

#ifdef __ANDROID__
static const char no_pack_in_apk[] = "";    /* (load_apk_pack: the APK carries no pack, a release's base APK) */

/* Android's pack, in the APK's assets (stored there by Simpsons APK, and by android.cmake in this computer's own):
   no file, so read whole through SDL's files and loaded from its bytes into the memories game.c's game_load fills
   (the programs' data as its pieces alone), as tmnt-port's. NULL, no_pack_in_apk, or what is wrong */
static const char *load_apk_pack(void)
{
	static const game_assets_memory into[GAME_ASSETS_REGIONS] = {
		[GAME_ASSETS_TILES] = { game_tiles, NULL }, [GAME_ASSETS_SPRITES] = { game_sprites, NULL },
		[GAME_ASSETS_SAMPLES] = { game_samples, NULL }, [GAME_ASSETS_EEPROM] = { game_eeprom, NULL } };
	SDL_RWops *packed = SDL_RWFromFile(GAME_ASSETS_PACK_FILE, "rb");
	const Sint64 size = packed ? SDL_RWsize(packed) : -1;
	uint8_t *bytes = size > 0 ? malloc((size_t)size) : NULL;
	size_t got = 0;
	const char *problem;

	if (!packed)
		return no_pack_in_apk;
	while (bytes && got < (size_t)size)
	{
		/* (its bytes as the asset manager hands them: maybe fewer than asked for at a time) */
		const size_t now = SDL_RWread(packed, bytes + got, 1, (size_t)size - got);

		if (now == 0)
			break;
		got += now;
	}
	SDL_RWclose(packed);
	problem = !bytes || got != (size_t)size ? "the assets pack cannot be read"
		: game_assets_pack_load_bytes_into(bytes, got, into);
	free(bytes);
	return problem;
}

#ifdef SIMPSONS_CONSOLE
/* The console's letters (font.bmp, Kenney Mini) out of the APK's assets into a folder of the app's own, which it
   returns (the caller frees it, SDL_free; NULL when it cannot): the console reads them with the C library's files,
   which do not reach the APK's assets. Their own folder, letters/ in the app's: SDL's files look at the app's folder
   before the APK's assets, so a font.bmp there would stand for the APK's */
static char *android_letters(void)
{
	char *saves = saves_folder(), *folder = NULL;
	char path[1200];
	SDL_RWops *from = SDL_RWFromFile("font.bmp", "rb"), *to = NULL;
	uint8_t chunk[16384];
	size_t count;
	bool whole = from != NULL;

	if (saves && from)
	{
		snprintf(path, sizeof(path), "%sletters", saves);
		mkdir(path, 0700);
		snprintf(path, sizeof(path), "%sletters/font.bmp", saves);
		to = SDL_RWFromFile(path, "wb");
	}
	while (to && (count = SDL_RWread(from, chunk, 1, sizeof(chunk))) > 0)
		whole = whole && SDL_RWwrite(to, chunk, 1, count) == count;
	if (to)
		whole = SDL_RWclose(to) == 0 && whole;
	if (from)
		SDL_RWclose(from);
	if (to && whole)
	{
		snprintf(path, sizeof(path), "%sletters/", saves);
		folder = SDL_strdup(path);
	}
	SDL_free(saves);
	return folder;
}
#endif
#endif

/* A device's assets loaded: the pack its package carries (the Switch's romfs: Simpsons NSP's, or an .nro made with
   the player's pack; the APK's: Simpsons APK's), else device_assets'; none, and the game says what to do and where
   (fail). '*problem' is NULL */
static bool find_assets(char *found, size_t size, const char **problem)
{
	static char message[1400];

	*problem = NULL;
#ifdef __SWITCH__
	snprintf(found, size, "romfs:/" GAME_ASSETS_PACK_FILE);
	if (exists(found))
	{
		const char *failed = game_load(found);

		if (!failed)
		{
			fprintf(stderr, "Assets: %s\n", found);
			return true;
		}
		fprintf(stderr, "%s: %s\n", found, failed);    /* (another version's: the device's folder's, if any) */
	}
#endif
#ifdef __WINRT__
	{
		/* (the package's own pack, put in by uwp.cmake where the build had the player's: the installed folder) */
		char *base = SDL_GetBasePath();

		snprintf(found, size, "%s" GAME_ASSETS_PACK_FILE, base ? base : "");
		SDL_free(base);
		if (exists(found))
		{
			const char *failed = game_load(found);

			if (!failed)
			{
				fprintf(stderr, "Assets: %s\n", found);
				return true;
			}
			fprintf(stderr, "%s: %s\n", found, failed);    /* (another version's: LocalState's, if any) */
		}
	}
#endif
#ifdef __ANDROID__
	{
		const char *failed = load_apk_pack();

		snprintf(found, size, "the APK's " GAME_ASSETS_PACK_FILE);
		if (!failed)
		{
			fprintf(stderr, "Assets: %s\n", found);
			return true;
		}
		/* (the APK is made on a computer, Simpsons APK putting the player's assets in it, Sebas: "se genera el apk
		   desde la computadora": a release's base APK, or one of another version, says so and nothing more) */
		if (failed != no_pack_in_apk)
			fprintf(stderr, "%s: %s\n", found, failed);    /* (another version's) */
		snprintf(message, sizeof(message), "This APK does not carry the game's assets%s: make it on a computer with "
			"Simpsons APK (in the release's simpsons-android.zip) from your " ROM_SET_FILE " (MAME's simpsons set, 4 "
			"Players World), then install the APK it makes.", failed != no_pack_in_apk ? " of this version" : "");
		fail(message);
	}
#endif
	if (!device_assets(found, size, message, sizeof(message)))
		fail(message);
	fprintf(stderr, "Assets: %s\n", found);
	return true;
}
#else
/* The folder 'assets' tools/extract_assets.py writes (for development), whole, into 'path': beside the executable
   ('base') or in one of the three folders above it (build/bin's executable finds the repository's), else in the
   current folder. false when there is none */
static bool find_assets_folder(const char *base, char *path, size_t size)
{
	char folder[1024];

	snprintf(folder, sizeof(folder), "%s", base ? base : "");
	for (int up = 0;; up++)
	{
		/* (the files every extraction writes now: an older, incomplete one is passed over) */
		static const char *const k_needed[] = { "data/layout.txt", "data/structures.txt", "sound_data/layout.txt",
			"eeprom.txt" };
		size_t length;
		bool complete = true;

		for (size_t file = 0; file < sizeof(k_needed) / sizeof(k_needed[0]) && complete; file++)
		{
			snprintf(path, size, "%sassets/%s", folder, k_needed[file]);
			complete = exists(path);
		}
		if (complete)
		{
			snprintf(path, size, "%sassets", folder);
			return true;
		}
		if (!folder[0])
			return false;    /* (the current folder's too) */
		/* the folder above: drop the last name and its separator; after the third, the current folder */
		length = strlen(folder);
		if (length && (folder[length - 1] == '/' || folder[length - 1] == '\\'))
			folder[--length] = 0;
		while (length && folder[length - 1] != '/' && folder[length - 1] != '\\')
			folder[--length] = 0;
		if (up == 3)
			folder[0] = 0;
	}
}

/* The game's assets loaded (game_load) from where 'found' says: the pack beside the executable (beside the .app on
   macOS) or in the folder of what the game keeps; else the folder 'assets' (find_assets_folder); else the first
   run's pack, made from the player's ROM set in the folder of what the game keeps (first_run.c), as it is when a
   pack found cannot be loaded (another version's). '*problem': NULL, or what went wrong loading 'found'. false when
   the first run's window was closed before a ROM set came */
static bool find_assets(char *found, size_t size, const char **problem)
{
	char *base = SDL_GetBasePath(), *saves = saves_folder();
	char beside_app[1100], stale[400] = "";
	const bool in_app = app_folder(base, beside_app, sizeof(beside_app)) != 0;
	const char *places[3];
	size_t count = 0;
	bool ready = false;

	*problem = NULL;
	places[count++] = base ? base : "";
	if (in_app)
		places[count++] = beside_app;
	if (saves && (!base || strcmp(base, saves) != 0))
		places[count++] = saves;
	for (size_t place = 0; place < count && !ready; place++)
	{
		snprintf(found, size, "%s" GAME_ASSETS_PACK_FILE, places[place]);
		if (!exists(found))
			continue;
		if ((*problem = game_load(found)) == NULL)
			ready = true;
		else
		{
			/* (another version's, or not whole: made again from the ROM set, unless a good one is further on) */
			fprintf(stderr, "%s: %s\n", found, *problem);
			snprintf(stale, sizeof(stale), "%s", *problem);
			*problem = NULL;
		}
	}
	if (!ready && find_assets_folder(base, found, size))
	{
		*problem = game_load(found);
		ready = true;
	}
	if (!ready)
	{
		snprintf(found, size, "%s" GAME_ASSETS_PACK_FILE, saves ? saves : (base ? base : ""));
		if (first_run(TITLE, base, in_app ? beside_app : NULL, saves, found, stale[0] ? stale : NULL, rom_set_option))
		{
			*problem = game_load(found);
			ready = true;
		}
	}
	if (ready && !*problem)
		fprintf(stderr, "Assets: %s\n", found);
	SDL_free(base);
	SDL_free(saves);
	return ready;
}

/* --prepare: the pack made with no window, as the first run makes it (first_run_found: from the ROM set --game names,
   else simpsons.zip or the folder rom beside the game or in the folder of what it keeps), into the folder of what the
   game keeps, and loaded once to check it; for a launcher's setup step, which shows the lines "progress FRACTION TEXT"
   of the standard output. With no ROM set anywhere the pack already there will do, if it loads. 0 when the pack is
   ready, 2 when it is not (what is wrong on both outputs) */
static int prepare(void)
{
	char *base = SDL_GetBasePath(), *saves = saves_folder();
	char beside_app[1100], pack[1100], problem[1400];
	const bool in_app = app_folder(base, beside_app, sizeof(beside_app)) != 0;
	const char *failed;
	bool ready;

	snprintf(pack, sizeof(pack), "%s" GAME_ASSETS_PACK_FILE, saves ? saves : (base ? base : ""));
	printf("progress 0.1 Reading the ROM set\n");
	fflush(stdout);
	ready = first_run_found(rom_set_option, base, in_app ? beside_app : NULL, saves, pack, problem, sizeof(problem));
	if (!ready && !problem[0] && exists(pack))
		ready = true;    /* (no ROM set, but the pack made of one before: if it loads) */
	if (ready)
	{
		printf("progress 0.8 Checking the game's assets\n");
		fflush(stdout);
		if ((failed = game_load(pack)) != NULL)
		{
			snprintf(problem, sizeof(problem), "%s: %s", pack, failed);
			fprintf(stderr, "%s\n", problem);
			ready = false;
		}
	}
	else if (!problem[0])
	{
		snprintf(problem, sizeof(problem), "No ROM set: name simpsons.zip (MAME's simpsons set, 4 Players World) with "
			"--game, or put it beside the game.");
		fprintf(stderr, "%s\n", problem);
	}
	if (ready)
	{
		fprintf(stderr, "Assets: %s\n", pack);
		printf("progress 1 The game's assets are ready\n");
	}
	else
		printf("%s\n", problem);    /* (a set's own problem is in the log already: first_run.c's say_problem) */
	fflush(stdout);
	SDL_free(base);
	SDL_free(saves);
	return ready ? 0 : 2;
}
#endif

static FILE *open_output(const char *path)
{
	FILE *file = fopen(path, "w");

	if (!file)
	{
		fprintf(stderr, "%s: cannot be written\n", path);
		exit(1);
	}
	return file;
}

#ifndef SIMPSONS_HANDHELD
/* RecompLauncher's Big Picture, its full-screen view for a controller and the couch, starts the game (its
   RECOMPLAUNCHER_MODE): the game full screen too, as players there expect. Not in a scripted run */
static bool launcher_big_picture(void)
{
	const char *mode = getenv("RECOMPLAUNCHER_MODE");

	return !scripted && mode && strcmp(mode, "bigpicture") == 0;
}
#endif

static void fail(const char *message)
{
	fprintf(stderr, "%s\n", message);
#ifdef __SWITCH__
	{
		/* (the Switch's SDL has no message box: libnx's text console, till + is pressed, as tmnt-port's. Not the
		   system's error dialog: from 10.0.0 it is a fatal error out of an application, as an .nro from the album) */
		PadState pad;

		consoleInit(NULL);
		padConfigureInput(1, HidNpadStyleSet_NpadStandard);
		padInitializeDefault(&pad);
		printf("%s\n\n%s\n\nPress + to leave.\n", TITLE, message);
		while (appletMainLoop())
		{
			padUpdate(&pad);
			if (padGetButtonsDown(&pad) & HidNpadButton_Plus)
				break;
			consoleUpdate(NULL);
		}
		consoleExit(NULL);
	}
#else
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, message, NULL);
#endif
	exit(1);
}

int main(int argc, char **argv)
{
	char assets_path[1024];
	const char *assets = NULL;
	const char *script_path = NULL;
	bool service = false;
#ifndef SIMPSONS_HANDHELD
	bool prepare_option = false;       /* --prepare */
#endif
	char message[1200];
	int scale = WINDOW_SCALE;
#ifdef SIMPSONS_CONSOLE
	int wide_option = -1;              /* --wide */
	bool stereo_option = false;        /* --stereo */
	const char *lan_name_option = NULL;    /* --lan-name */
	bool arcade_start = false;         /* --arcade-start */
	const char *scores_option = NULL;  /* --scores */
	const char *state_option = NULL;   /* --state */
	char scores_path[1100] = "";
#endif
#ifdef __SWITCH__
	char taken[300];
	const int taken_count = switch_take_old_folder(taken, sizeof(taken));    /* (before open_log makes the folder) */
#endif

	open_log();
#ifdef __SWITCH__
	if (taken_count > 0)
		fprintf(stderr, "From " SWITCH_OLD_FOLDER "/ (1.0.0's folder) into " SWITCH_FOLDER "/: %s\n", taken);
	romfsInit();    /* (the assets pack in the game's romfs: the .nsp's, or an .nro made with the player's pack) */
#endif
	for (int index = 1; index < argc; index++)
	{
		if (strcmp(argv[index], "--script") == 0 && index + 1 < argc)
			script_path = argv[++index];
		else if (strcmp(argv[index], "--fast") == 0)
			fast = true;
		else if (strcmp(argv[index], "--hashes") == 0 && index + 1 < argc)
			hashes = open_output(argv[++index]);
		else if (strcmp(argv[index], "--video-hashes") == 0 && index + 1 < argc)
			video_hashes = open_output(argv[++index]);
		else if (strcmp(argv[index], "--sound-hashes") == 0 && index + 1 < argc)
			sound_hashes = open_output(argv[++index]);
		else if (strcmp(argv[index], "--service") == 0)
			service = true;
		else if (strcmp(argv[index], "--slow") == 0 && index + 1 < argc)
			slow_ms = atoi(argv[++index]);
		else if (strcmp(argv[index], "--no-vsync") == 0)
			no_vsync = true;
#if defined(SIMPSONS_PAD_CHECK) && defined(SIMPSONS_CONSOLE)
		else if (strcmp(argv[index], "--pads") == 0 && index + 1 < argc)
		{
			const int count = atoi(argv[++index]);    /* (by itself: SDL_clamp is a macro) */

			check_pads = SDL_clamp(count, 0, PLAYERS);
		}
#endif
		else if (strcmp(argv[index], "--assets") == 0 && index + 1 < argc)
			assets = argv[++index];
		else if (strcmp(argv[index], "--video-dir") == 0 && index + 1 < argc)
			dump_dir = argv[++index];
		else if (strcmp(argv[index], "--video-dump") == 0 && index + 1 < argc)
		{
			while (index + 1 < argc && argv[index + 1][0] != '-' && dump_count < MAX_DUMPS)
				dump_frames[dump_count++] = strtoull(argv[++index], NULL, 10);
		}
#ifndef SIMPSONS_HANDHELD
		else if (strcmp(argv[index], "--game") == 0 && index + 1 < argc)
			rom_set_option = argv[++index];
		else if (strncmp(argv[index], "--game=", 7) == 0 && argv[index][7])
			rom_set_option = argv[index] + 7;    /* (one argument: a launcher leaves it out whole without a path) */
		else if (strcmp(argv[index], "--prepare") == 0)
			prepare_option = true;
#endif
#ifdef SIMPSONS_CONSOLE
		else if (strcmp(argv[index], "--actions") == 0 && index + 1 < argc)
		{
			const char *problem = load_actions(argv[++index]);

			if (problem)
				fail(problem);
			scripted = true;
		}
		else if (strcmp(argv[index], "--wide") == 0 && index + 1 < argc)
		{
			const char *kind = argv[++index];

			wide_option = -1;
			for (int known = 0; known < WIDE_KINDS; known++)
				if (strcmp(kind, console_wide_names[known]) == 0 || (kind[0] == '0' + known && !kind[1]))
					wide_option = known;
			if (wide_option < 0)
				fail("WIDE is one of off, 16:10, 16:9, 20:9, auto (or 0-4).");
		}
		else if (strcmp(argv[index], "--wide-check") == 0)
			wide_check = true;
		else if (strcmp(argv[index], "--arcade-start") == 0)
			arcade_start = true;
		else if (strcmp(argv[index], "--stereo") == 0)
			stereo_option = true;    /* (SOUND: STEREO, for the checks) */
		else if (strcmp(argv[index], "--lan-name") == 0 && index + 1 < argc)
			lan_name_option = argv[++index];    /* (MULTIPLAYER > NAME, for the checks) */
		else if (strcmp(argv[index], "--lan-hashes") == 0 && index + 1 < argc)
		{
			if ((lan_hashes = fopen(argv[++index], "w")) == NULL)
				fail("The LAN hashes' file cannot be written.");
		}
		else if (strcmp(argv[index], "--scores") == 0 && index + 1 < argc)
			scores_option = argv[++index];
		else if (strcmp(argv[index], "--state") == 0 && index + 1 < argc)
			state_option = argv[++index];
		else if (strcmp(argv[index], "--state-save") == 0 && index + 1 < argc)
			state_save_at = strtoull(argv[++index], NULL, 10);
		else if (strcmp(argv[index], "--state-load") == 0)
			state_load_pending = true;
#else
		else if (strcmp(argv[index], "--eeprom") == 0 && index + 1 < argc)
			eeprom_option = argv[++index];
#endif
#ifdef SIMPSONS_TOUCH_CHECK
		else if (strcmp(argv[index], "--screen") == 0 && index + 1 < argc)
		{
			if (sscanf(argv[++index], "%dx%d", &check_screen_width, &check_screen_height) != 2
				|| check_screen_width <= 0 || check_screen_height <= 0)
				fail("--screen is WIDTHxHEIGHT, a phone's or a tablet's screen in pixels.");
		}
#endif
		else
			fail(USAGE);
	}
	scripted = scripted || script_path || hashes || video_hashes || sound_hashes || dump_count;
#ifndef SIMPSONS_HANDHELD
	if (prepare_option)
		return prepare();    /* (the pack alone, no window: a launcher's setup step) */
#endif
#ifdef _WIN32
	/* Direct3D 11 for the picture on Windows (SDL 2's first is Direct3D 9: Sebas, on tmnt-port, "pone por default que
	   use dx11"): its swap chain is DXGI's flip model, the frames' pace steadier in a window with VSYNC, and Alt+Tab or
	   the full screen lose none of its textures. SDL takes the others where it cannot start, and SDL_RENDER_DRIVER in
	   the environment still chooses; a renderer named by the hint has SDL's batching off unless it is asked for */
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
	SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
	/* The screen's own pixels where Windows scales the desktop (the players' issue 12: a 3840 x 2160 screen at 225 %
	   gave the game 1707 x 960, which Windows stretched with its bilinear): the game DPI-aware, SDL's windows in the
	   desktop's points (as large as they were) and their pictures in the screen's pixels (present draws in the
	   renderer's output, its pixels; the first run's window keeps its layout, first_run.c) */
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
#endif
#ifdef __WINRT__
	/* (the Xbox controller's B is the system's back too: marked handled, so the app is never sent away by it; its key,
	   SDLK_AC_BACK, is not the menus' back here, as the controller's B already is what CONTROLS makes it) */
	SDL_SetHint(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, "1");
#endif
#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");    /* (back is the menus' back and the pause, never the end) */
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");          /* (fingers are the touch controls, not the mouse) */
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");   /* (the phone's tilt is no game controller) */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");          /* (fingers are the touch controls, not the mouse) */
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");   /* (the phone's tilt is no game controller) */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");     /* (dimmed, and a first swipe from the bottom only shows it) */
	SDL_SetHint(SDL_HINT_AUDIO_CATEGORY, "playback");       /* (AVAudioSession's playback: heard with the ringer off) */
	SDL_SetHint(SDL_HINT_IDLE_TIMER_DISABLED, "1");         /* (no dimming nor locking while a game controller plays) */
#endif
	add_mappings();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
	{
		snprintf(message, sizeof(message), "The window cannot be opened: %s", SDL_GetError());
		fail(message);
	}
	fprintf(stderr, "Game controllers: %d mappings\n", SDL_GameControllerNumMappings());
#if defined(SIMPSONS_PAD_CHECK) && defined(SIMPSONS_CONSOLE)
	for (int pad = 0; pad < check_pads; pad++)
	{
		/* (plugged in: controller_added, as any other) */
		const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, SDL_CONTROLLER_AXIS_MAX,
			SDL_CONTROLLER_BUTTON_MAX, 0);

		if (index < 0)
			fprintf(stderr, "--pads: %s\n", SDL_GetError());
		else
			fprintf(stderr, "--pads: SDL's virtual joystick %d, %s\n", index,
				SDL_IsGameController(index) ? "a game controller" : "no game controller");
	}
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
	SDL_AddEventWatch(ios_app_events, NULL);    /* (the background, as it comes: nothing drawn there) */
#endif
	unseen = fast && SDL_GetCurrentVideoDriver() && strcmp(SDL_GetCurrentVideoDriver(), "dummy") == 0;
#ifdef SIMPSONS_TOUCH_CHECK
	unseen = unseen && check_screen_width <= 0;    /* (--screen: the screens presented are written, check_screen_dump) */
#endif
	{
		/* (the folder of what the game keeps: its settings, each player's controls, the arcade's EEPROM. A scripted
		   run takes the defaults and the factory's, and writes none of them) */
		char *saves = saves_folder();
		char controls_path[1100];

		snprintf(controls_path, sizeof(controls_path), "%s" CONTROLS_FILE, saves ? saves : "");
		controls_open(scripted ? "" : controls_path);
#ifdef SIMPSONS_CONSOLE
		if (!scripted)
		{
			snprintf(settings_path, sizeof(settings_path), "%s" SETTINGS_FILE, saves ? saves : "");
			load_settings();
#ifndef SIMPSONS_HANDHELD
			if (launcher_big_picture() && !console_settings.full_screen)
			{
				console_settings.full_screen = true;    /* (this time: simpsons.ini keeps its window, save_settings) */
				launcher_full_screen = true;
			}
#endif
			snprintf(scores_path, sizeof(scores_path), "%s" SCORES_FILE, saves ? saves : "");
			snprintf(state_path, sizeof(state_path), "%s" STATE_FILE, saves ? saves : "");
		}
		if (scores_option)
			snprintf(scores_path, sizeof(scores_path), "%s", scores_option);    /* (--scores: a check's) */
		if (state_option)
			snprintf(state_path, sizeof(state_path), "%s", state_option);    /* (--state: a check's) */
		if ((state_save_at || state_load_pending) && !state_path[0])
			fail("--state-save and --state-load need --state FILE.");
		if (wide_option >= 0)
			console_settings.wide = wide_option;
		if (stereo_option)
			console_settings.stereo = true;
		if (lan_name_option)
			snprintf(console_settings.name, sizeof(console_settings.name), "%s", lan_name_option);
		game_set_stereo(console_settings.stereo);
		scale = console_settings.window_scale;
#ifdef SIMPSONS_HANDHELD
		console_settings.full_screen = true;    /* (a device's screen, whole: apply_settings never makes it a window) */
#endif
#ifdef SIMPSONS_SCREEN_WIDE
		console_settings.wide = WIDE_16_9;    /* (a device's screen: 16:9, its picture fixed) */
#endif
#else
		if (eeprom_option)
			snprintf(eeprom_path, sizeof(eeprom_path), "%s", eeprom_option);    /* (--eeprom: a check's, kept) */
		else if (!scripted)
			snprintf(eeprom_path, sizeof(eeprom_path), "%s" EEPROM_FILE, saves ? saves : "");
#endif
		SDL_free(saves);
	}
	{
		const char *problem;

		if (assets)
			problem = game_load(assets);
		else if (!find_assets(assets_path, sizeof(assets_path), &problem))
		{
			/* (the first run's window closed before a ROM set came) */
			SDL_Quit();
			return 0;
		}
		else
			assets = assets_path;
		if (problem)
		{
			snprintf(message, sizeof(message), "The game's assets in %s cannot be read: %s", assets, problem);
			fail(message);
		}
	}
	if (script_path)
	{
		char error[256];

		script = simpsons_script_load(script_path, error, sizeof(error));
		if (!script)
			fail(error);
		script_frames = simpsons_script_frames(script);
	}

#ifdef __SWITCH__
	(void)scale;
	window = SDL_CreateWindow(TITLE, 0, 0, 1280, 720, 0);    /* (the Switch's screen, whole) */
#elif defined(SIMPSONS_HANDHELD_KEYS)
	(void)scale;
	window = SDL_CreateWindow(TITLE, 0, 0, 640, 480, SDL_WINDOW_FULLSCREEN_DESKTOP);    /* (the handheld's, whole) */
#elif defined(__ANDROID__)
	(void)scale;
	/* (the phone's whole screen: full screen is SDL's immersive mode, the system's bars hidden. A window would leave
	   them over the picture, Android 15 drawing the app under them: tmnt-port's player's OnePlus 12, the score cut) */
	window = SDL_CreateWindow(TITLE, 0, 0, 1280, 720, SDL_WINDOW_FULLSCREEN_DESKTOP);
#elif defined(__WINRT__)
	(void)scale;
	/* (the TV's whole screen at its own pixels: the Xbox's 1920 x 1080 swap chain, a computer's whole window) */
	window = SDL_CreateWindow(TITLE, 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI);
#elif defined(__APPLE__) && TARGET_OS_IOS
	(void)scale;
	/* (the whole screen at its own pixels (ALLOW_HIGHDPI: the touch controls' size by the screen's density, touch.c,
	   in the same pixels as SDL_GetDisplayDPI's), the status bar hidden (BORDERLESS)) */
	window = SDL_CreateWindow(TITLE, 0, 0, 1280, 720, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS
		| SDL_WINDOW_ALLOW_HIGHDPI);
#else
#ifdef SIMPSONS_TOUCH_CHECK
	if (check_screen_width > 0)
		window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, check_screen_width,
			check_screen_height, 0);    /* (a check's phone or tablet screen) */
	else
#endif
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		view_width() * scale, SIMPSONS_VIDEO_HEIGHT * scale, SDL_WINDOW_RESIZABLE);
#endif
	renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE) : NULL;
	if (!renderer)
		renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE) : NULL;
	{
		SDL_RendererInfo info;

		if (renderer && SDL_GetRendererInfo(renderer, &info) == 0)
			fprintf(stderr, "Renderer: %s\n", info.name);    /* (the log says which one came: a player's question) */
#ifndef SIMPSONS_HANDHELD
		if (renderer)
		{
			/* (the window and the pixels its picture has: the same but where the desktop is scaled, issue 12. The
			   computers': the Anbernic's stub has no SDL_GetWindowSize) */
			int points = 0, points_high = 0, pixels = 0, pixels_high = 0;

			SDL_GetWindowSize(window, &points, &points_high);
			SDL_GetRendererOutputSize(renderer, &pixels, &pixels_high);
			fprintf(stderr, "Window: %dx%d, its picture %dx%d pixels\n", points, points_high, pixels, pixels_high);
		}
#endif
	}
#ifdef SIMPSONS_CONSOLE
	texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
		SIMPSONS_VIDEO_MAX_WIDTH, SIMPSONS_VIDEO_HEIGHT) : NULL;    /* (WIDE's frames at its left) */
#else
	texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
		SIMPSONS_VIDEO_WIDTH, SIMPSONS_VIDEO_HEIGHT) : NULL;
#endif
	if (!texture)
	{
		snprintf(message, sizeof(message), "The window cannot be opened: %s", SDL_GetError());
		fail(message);
	}
	SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
#if !defined(SIMPSONS_CONSOLE) && !defined(SIMPSONS_HANDHELD)
	if (launcher_big_picture())
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);    /* (The Simpsons': its settings, above) */
#endif
#ifdef SIMPSONS_TOUCH
	{
#ifdef __ANDROID__
		char *base = SDL_strdup("");    /* (the APK's assets, which SDL's files read by a relative path) */
#else
		char *base = SDL_GetBasePath();
#endif

		touch_open(renderer, base ? base : "");
		SDL_free(base);
	}
#endif
#ifdef SIMPSONS_CONSOLE
	{
		/* WIDE AUTO as the screen is at the start (follow_screen: again as it changes; a phone's screen not known
		   yet, 16:9), the window sized for it */
		int out_width = 0, out_height = 0;

		SDL_GetRendererOutputSize(renderer, &out_width, &out_height);
#ifdef SIMPSONS_TOUCH
		if (out_height <= 0)
			console_set_screen_wide(WIDE_16_9);
#endif
		follow_screen();
	}
#endif
	SDL_RaiseWindow(window);

	{
		const char *problem = game_reset(service, true);

		if (problem)
		{
			snprintf(message, sizeof(message), "The game does not start: %s", problem);
			fail(message);
		}
	}
#ifdef SIMPSONS_CONSOLE
	{
		/* (the menus' letters and the prompts beside the executable; OPTIONS' cabinet settings into the EEPROM) */
#ifdef __ANDROID__
		char *base = android_letters();    /* (font.bmp out of the APK into the app's folder: the console reads a file) */
#else
		char *base = SDL_GetBasePath();
#endif

		if (!console_attach(base))
			fprintf(stderr, "%sfont.bmp cannot be read\n", base ? base : "");
		if (arcade_start)
			console_arcade_test();    /* (the arcade's scenarios in the console's build) */
		console_scores_open(scores_path);    /* (the high scores kept; none in a scripted run but --scores') */
#ifdef __ANDROID__
		load_prompts(NULL);    /* (the APK's assets, through SDL's files) */
#else
		load_prompts(base);
#endif
		SDL_free(base);
		apply_settings();
	}
#else
	if (eeprom_path[0])
	{
		load_eeprom();    /* (the factory's from the assets where there is none: game_reset's) */
		simpsons_eeprom_written = eeprom_written;
	}
	apply_vsync();
#endif
	service_held = service;
	open_audio();
	if (script)
		simpsons_script_apply(script, 0);
	game_frame_done = on_frame_done;
#ifdef SIMPSONS_CONSOLE
	if (setjmp(lan_restart_point))
	{
		/* (MULTIPLAYER's game starts: the board reset, the console as at power-on with the host's settings, the
		   program from its start, on every device alike; as segasonic-port's) */
		const char *problem = game_reset(false, true);

		if (problem)
			fail(problem);
		console_lan_begin();
		if (audio)
			SDL_ClearQueuedAudio(audio);
		next_frame = 0;
		pause_requested = false;
	}
#endif
	program_run();
	quit();
	return 0;
}
