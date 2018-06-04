# wkbre

![Banner](../gh-pages/images/header-bg.jpg)

__wkbre__ is an engine reimplementation of __Warrior Kings (Battles)__ (a 3D RTS game by Black Cactus released in 2002-2003), now released under the __GPL3__ license. It can open the game data files, load the levels, make changes to them and render the game world.

[Download binaries](https://github.com/AdrienTD/wkbre/releases)

While the ability to play games in the engine is currently in development, wkbre can still be used as a __level editor__. You can open savegames in it, make some changes like creating, moving or deleting characters, buildings, ..., then save the changes and load the new savegame in the original game engine. It also has a __terrain editor__ and you can make new custom skirmish maps that you can play in the original game alone or with friends!

You need a copy of the game if you want to use wkbre. The game is still available on Steam ([WK](http://store.steampowered.com/app/297570) / [WKB](http://store.steampowered.com/app/299070)) and GOG ([WK](https://www.gog.com/game/warrior_kings) / [WKB](https://www.gog.com/game/warrior_kings_battles)).

Currently, wkbre only works on __Windows__ (XP, Vista, 7, 8, 8.1, 10, and later). But a port to other OSs is planned.

You can watch some videos of wkbre in action on [my YouTube channel](https://www.youtube.com/user/ItsAdrienG).

## Compiling

I have only tested the source code on Visual C++ 2010/2017 and MinGW.

You can use [CMake](https://cmake.org) to generate a Makefile or a project file (for example a Visual Studio solution). In Visual Studio 2017 you can even open the CMakeLists.txt file as a project file and directly build from there.

Or, you can also build the program with the included batch files:

* Open a Windows SDK/Visual Studio/MinGW-32 command prompt.
* cd to wkbre's source code directory.
* cd to bzip2
* Type (before pressing ENTER)
  * `build_vc_d` if you use Microsoft Visual C++ compiler
  * `build_mg_d` if you use MinGW
* cd ..
* Type (before pressing ENTER)
  * `vc_compall` if you use Microsoft Visual C++ compiler
  * `mg_compall` if you use MinGW

For more information see [docs/codedoc.txt](docs/codedoc.txt).

## Execution
To run wkbre, you must first open **settings.txt** and change the **GAME_DIR** value to the path of WK's game data (the directory with data.bcp and saved directory). Alternatively, you can comment the **GAME_DIR** variable out and __copy__ the wkbre **exe** as well as the **redata** directory in the game's directory.

If you start a development version of wkbre, you will be asked to enter a test number. Simply press ENTER to run the default main code.

For more information see [docs/help.htm](docs/help.htm).

## Third-party libraries used
* [LZRW3 compression/decompression library](http://www.ross.net/compression/lzrw3.html) (public domain)
* [bzip2 compression/decompression library](http://www.bzip.org) (bzip2 license)
* [Dear ImGui GUI library](https://github.com/ocornut/imgui) (MIT license)

These libraries are already included in wkbre's source repository, so you don't have to download them separately.

Their licenses files can be found in their respective directories.

## Conclusion
I hope you find wkbre and its source code interesting and useful. Comments/suggestions/bug reports are really appreciated.

I also made a __"WK Modding kit"__ with some programs for viewing and editing game files. You can download it [here](https://github.com/AdrienTD/wktools).

## Links
* [Warrior Kings Community Steam group](http://steamcommunity.com/groups/WARKC)
* [Warrior Kings Community Discord](https://discord.gg/dkb7APh)
* [Warrior Kings Forum](http://wkforums.de.to)
* [Warrior Kings modding information site](https://sites.google.com/site/wkmodding/)
* [WK Modding Kit](https://github.com/AdrienTD/wktools)
