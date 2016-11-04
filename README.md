# wkbre

![Banner](../gh-pages/images/header-bg.jpg)

__wkbre__ is an engine reimplementation of __Warrior Kings (Battles)__ (a 3D RTS game by Black Cactus released in 2002-2003), now released under the __GPL3__ license. It can open the game data files, load the levels and render the game world.

[Download binaries](https://github.com/AdrienTD/wkbre/releases)

While the ability to play games in the engine is currently in development, wkbre can still be used as a level editor. You can open savegames in it, make some changes like creating, moving or deleting characters, buildings, ..., then save the changes and load the new savegame in the original game engine.

You need a copy of the game if you want to execute wkbre. The game is still available on Steam ([WK](http://store.steampowered.com/app/297570) / [WKB](http://store.steampowered.com/app/299070)) and GOG ([WK](https://www.gog.com/game/warrior_kings) / [WKB](https://www.gog.com/game/warrior_kings_battles)).

Currently, wkbre only works on __Windows__ (XP, Vista, 7, 8, 8.1, 10, and later). But a port to other OSs is planned.

You can watch a trailer of wkbre on [YouTube](https://www.youtube.com/watch?v=8agit5vO6cw).
There's also a video of an older version of wkbre on [YouTube](https://www.youtube.com/watch?v=K2LLjLelEJA).

## Compilation
__Note:__ It is recommended to build a 32-bit executable, otherwise if you make a 64-bit executable it might not find all your savegames (if the game is in the "Program Files" directory and you often run the game without administrator rights).
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

(I know, I prefer batch files over Makefiles and project files. But later I may create some Makefiles.)

For more information see [docs/codedoc.txt](docs/codedoc.txt).

## Execution
To run wkbre, you must first open settings.txt and change the GAME_DIR value to the path of WK's game data (the directory with data.bcp and saved directory).

When you start wkbre, you might be asked to enter a test number. Simply press ENTER to run the default main code.

For more information see [docs/help.htm](docs/help.htm).

## Conclusion
I hope you find wkbre and its source code interesting and useful. Comments/suggestions/bug reports are really appreciated.

I also made a __"WK Modding kit"__ with some programs for viewing and editing game files. You can download it [here](https://github.com/AdrienTD/wktools).
