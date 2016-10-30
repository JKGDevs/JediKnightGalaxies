# Jedi Knight Galaxies #
[IRC](https://kiwiirc.com/client/irc.arloria.net/?nick=Padawan|?&theme=cli#jkgalaxies): irc.arloria.net / #JKGalaxies

FAQ: http://jkhub.org/topic/6613-jkg-faq/

Downloads:
[Binaries](http://jkhub.org/files/file/2544-jedi-knight-galaxies-binaries/) , [Assets](http://jkhub.org/files/file/2543-jedi-knight-galaxies-assets/) , 
[Map Bundle #1](http://jkhub.org/files/file/2652-jedi-knight-galaxies-map-bundle-1/)

Forum: http://www.tinyurl.com/JKGJKH


Forum: https://jkhub.org/forum/117-jedi-knight-galaxies/

Please use discretion when making issue requests on GitHub. The forum is a better place for larger discussions on changes that aren't actually bugs.

## Build Guide ##
https://jkhub.org/topic/5600-compilation-guide/

## Installation of Game ##
JKG is currently only available on Windows, but we have a mac version being worked on and hopefully a linux one soon.

First, you need Jedi Academy installed. If you don't already own the game you can buy it from online stores such as [Steam](http://store.steampowered.com/app/6020/), [Amazon](http://www.amazon.com/Star-Wars-Jedi-Knight-Academy-Pc/dp/B0000A2MCN) or [Play](http://www.play.com/Games/PC/4-/127805/Star-Wars-Jedi-Knight-Jedi-Academy/Product.html?searchstring=jedi+academy&searchsource=0&searchtype=allproducts&urlrefer=search).

There are a few ways to actually install JKG itself, the simplest is to just get the [binaries](http://jkhub.org/files/file/2544-jedi-knight-galaxies-binaries/) and [assets](http://jkhub.org/files/file/2543-jedi-knight-galaxies-assets/), and then follow the directions in the readme included.  Please note that while we try to keep these relatively up to date, the github will always contain the most current version of the assets and binaries and you can always compile and pack the pk3s yourself to get a more up to date version.

Alternatively you can install the assets and binaries inside a directory such as C:\JKG\ and then use a batch file/command line arguments to open jkgalaxies.x86.exe while pointing it to the Jedi Academy assets.  This is the recommended setup for developers, as you can then call your exe's location with appropriate args directly from your IDE.  You can do this by setting the fs_basepath arg to be equal to your Jedi Academy install location.  You'll also want to set the fs_cdpath to be equal to JKG's directory.  Optionally you can use fs_homepath to store screenshots and other user specific information, if not specified fs_homepath will be set to "...Documents\My Games\JKGalaxies"  For example, use this batch script on steam installs for windows:

	jkgalaxies.x86.exe +set fs_game "JKG" +set fs_cdpath "." +set fs_basepath "C:\Program Files (x86)\Steam\SteamApps\common\Jedi Academy\GameData" +set r_fullscreen 0


## Dependencies ##

* Boost library (only for some branches)
* SDL2 (2.0.3+) (included on Windows)
* OpenGL
* OpenAL (included on Windows)
* libpng (included on Windows)
* libjpeg (included on Windows)
* zlib (included on Windows)

## Dedicated Server ##

In order to run a dedicated server, you must use the JKGalaxiesDed binary. Running dedicated from the main executable is currently not possible because it was intentionally broken with the addition of modular renderer.

## Developer Notes ##

JKG is licensed under GPLv2 as free software. You are free to use, modify and redistribute JKG following the terms in [LICENSE.txt](https://github.com/JKGDevs/JediKnightGalaxies/blob/master/LICENSE.txt).

Please be aware of the implications of the GPLv2 licence. In short, be prepared to share your code under the same GPLv2 licence.  

### If you wish to contribute to JKGalaxies, please do the following ###
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github
* Create a new branch and make your changes
* Send a [pull request](https://help.github.com/articles/creating-a-pull-request) to upstream (JKGDevs/JediKnightGalaxies)

### If you wish to base your work off JKGalaxies (mod or engine) ###
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github
* Change the GAMEVERSION define in codemp/game/g_local.h from "jkgalaxies" to your project name
* If you make a nice change, please consider back-porting to upstream via pull request as described above. This is so everyone benefits without having to reinvent the wheel for every project.


## Current Project Goals ##
These are very much subject to change, especially phases later than Versus.

### Phase 1:  Versus (current iteration). ###
* The goal here is to have a functioning arena based shooter (similar to games such as Call of Duty or Unreal Tournament) with the beginnings of some later RPG elements such as armor, pazaak card game, etc.  There are three major milestones currently being worked on: [Milestone 2](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=issue+milestone%3A%22Versus+Revision+2%22), [Milestone 3](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=issue+milestone%3A%22Versus+Revision+3%22) and [Milestone 4](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=is%3Aopen+is%3Aissue+milestone%3A%22Versus+Revision+4%22).  Milestone 2 mainly features smaller bug fixes needed to make the game stable, while Milestone 3 includes new features like better melee, armor, shields, etc. in addition to some bug fixes not addressed in Milestone 2. Milestone 4 primarily focuses on the all new saber system and related features such as duel mode.

#### Phase 2: Coop ####
* The main features that are added here include the NPC system and fleshing out of the dialogue system. They also include overhauls to the chat systems and more.  We may also attempt some MoBA like features (still needs discussion).

#### Phase 3: RPG ####
* Open World RPG. The main features added here include quests, (more) minigames, and AI for existing minigames, like Pazaak. Also includes levelling up, experience, single person quests, large planet-themed maps, etc. This mode will mainly focus on implementing things from a single player's perspective.

#### Phase 4: LMO ####
* Features to the main game include group quests, Looking For Group system, and a lot of the dungeons that were designed for Phase 2 brought back. Includes a master server based architecture and functioning account system.  At this point, the game is pretty much done.
* Later possible expansions: space battles, more worlds, additional minigames




This project is based on [OpenJK](https://github.com/JACoders/OpenJK)
