The Simpsons on the Xbox (Developer Mode)
=========================================

The Simpsons Mr. Plow Edition and The Simpsons Arcade Game as UWP apps, for an Xbox One or Xbox Series X|S in
Developer Mode. You bring your own ROM set: MAME's simpsons set (4 Players World), as simpsons.zip.

What you need
-------------
- Your Xbox in Developer Mode (the Xbox Dev Mode Activation app; Microsoft's Partner Center developer account).
- A computer on the same network, with a web browser.
- The packages: SimpsonsMrPlow_<version>_x64.msix and/or SimpsonsArcade_<version>_x64.msix, each with its .cer
  and Dependencies\x64\Microsoft.VCLibs.x64.14.00.appx.

Installing
----------
1. On the Xbox, in Dev Home, turn on Remote Access (Device Portal) and note its address (https://<xbox-ip>:11443).
2. On the computer, open that address. Under "My games & apps", choose Add.
3. Choose the game's .msix. Next: add the dependency Microsoft.VCLibs.x64.14.00.appx (and the .cer if asked).
   Then Start. Do the same for the other game if you want both.
4. In the list, open the game's menu ("View details" or "Change view") and set its type to Game, not App: a game
   gets the whole console's power, an app only a share of it.

Your ROM set
------------
The packages carry no assets. Put simpsons.zip in the game's own folder, once for each game:
1. In Device Portal, open File explorer > LocalAppData > the game's package (SimpsonsPort.MrPlowEdition_... or
   SimpsonsPort.ArcadeGame_...) > LocalState.
2. Upload simpsons.zip there.
3. Start the game. Its first start makes the game's assets from it (a few seconds); the next starts are immediate.
If the game says it has no ROM set, it shows the folder it looked in.

The games keep their settings, high scores, saved state, controls and log (simpsons.log, simpsons-arcade.log) in
that same LocalState folder.

Playing
-------
Each controller is a player, up to four. The D-pad or the left stick moves, A attacks, B jumps, Menu (Start) is the
start button, View (Back) is the coin in The Simpsons Arcade Game and the pause in The Simpsons Mr. Plow Edition.
OPTIONS > CONTROLS in The Simpsons Mr. Plow Edition binds each player's buttons. EXIT in its menu leaves the game;
the Xbox button always takes you home.

Building the packages yourself (Windows)
----------------------------------------
Visual Studio 2022 with "Universal Windows Platform development" and its C++ (v143) UWP tools, then:
  cmake -S . -B build/uwp -G "Visual Studio 17 2022" -A x64 -DCMAKE_SYSTEM_NAME=WindowsStore
      -DCMAKE_SYSTEM_VERSION=10.0 -DSIMPSONS_REFERENCE_TOOLS=OFF
  cmake --build build/uwp --config Release --target simpsons_uwp_games
The packages land in build/bin/uwp/packages/. The first configure makes a signing key in local/uwp_key.pfx (kept,
never committed). Optional, before configuring:
- local/icon_uwp.png: a picture of your own for the tiles (needs Python 3 with Pillow).
- build/bin/simpsons.assets: your assets pack, carried inside the packages (they are then yours alone: never share
  them), so no simpsons.zip is needed on the Xbox.
