# Jedi Knight Galaxies #

### Links ###
[Webpage](https://www.jkgalaxies.net)  |  [FAQ](https://www.jkgalaxies.net/faq.html)
   

[![Github All Releases](https://img.shields.io/github/downloads/JKGDevs/JediKnightGalaxies/total.svg)](https://www.jkgalaxies.net/download.html)
[![discord](https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat)](https://discord.gg/YuG8Zks)
[![forum](https://img.shields.io/badge/forum-JKHub.org-brightgreen.svg)](https://jkhub.org/forums/forum/117-jedi-knight-galaxies/)
[![build](https://github.com/JKGDevs/JediKnightGalaxies/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=develop)](https://github.com/JKGDevs/JediKnightGalaxies/actions/workflows/cmake-multi-platform.yml?query=branch%3Adevelop)  

[YouTube](https://www.youtube.com/channel/UCnnAUSngUk8l3fycYMVXkKQ/featured)  
[Twitch](https://www.twitch.tv/jkgalaxies) 
[ModDB Page](http://www.moddb.com/mods/jkgalaxies) (not updated frequently)  
[Odysee.com](https://odysee.com/@JediKnightGalaxies:c?view=about) (not updated frequently)  
[Bluesky](https://bsky.app/profile/jkgalaxies.net) (not updated frequently)  


### What is Jedi Knight Galaxies? ###
Jedi Knight Galaxies is a competitive and innovative multiplayer shooter, played out in the Star Wars universe. Currently a full conversion mod, the project was originally based on and modified from Raven Software’s [Jedi Knight: Jedi Academy](https://en.wikipedia.org/wiki/Star_Wars_Jedi_Knight:_Jedi_Academy). Jedi Knight Galaxies is open source and available completely free of charge; however, you will need the original game in order to play. (Jedi Academy is frequently available on [Steam](http://store.steampowered.com/app/6020/STAR_WARS_Jedi_Knight__Jedi_Academy/) and [GoG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy) during sales for less than $4). We aim to develop the project into a Large Multiplayer Online game, crossing traditional RPG (role playing game) elements with the fast paced excitement of an FPS (first person shooter).

The project will be released in [phases](https://github.com/JKGDevs/JediKnightGalaxies/#phase-1--versus-current-iteration), each building on to the previous one with more aspects of the final goal. The current Phase, JKG: Versus, was first released September 17th, 2012 as a public beta, showcasing the primary FPS elements of the game. Subsequent updates and versions have been produced since then. As development progresses, sandbox environments will be added, initial RPG elements will appear, and player vs. enemy combat will be implemented into the faction oriented gameplay. Improved dialog and cinematic features along with skills, powers, and more aspects that belong in a Role-Playing Game will be developed, set between the events of Episode IV: A New Hope and Episode III: Revenge of the Sith. Finally, the full-fledged JKG: LMO takes the experience to a much grander level.

The game is a much loved hobby project supported by its [community and developers](https://www.jkgalaxies.net/contributors.html) who work on it in their freetime, contribution is encouraged and newbies are welcome. Originally founded by a group of Jedi Academy modders led by Jake (now retired), the project has since been in and out of development since 2008. The current development team comes from a variety of backgrounds, countries, and age, united by our desire to make a great game.

Welcome to Jedi Knight Galaxies, where we break the limits!


#### Trailer ####
[![JKG v1.3.22 Trailer](http://img.youtube.com/vi/X63qy7lONyo/0.jpg)](http://www.youtube.com/watch?v=X63qy7lONyo "JKG v1.3.22 Trailer")


### Downloads ###
The most up to date versions are available on the download page on the website, under the [test releases section](https://www.jkgalaxies.net/download.html#testdiv), but you can also use the following direct links (not frequently updated - may be out of date): [Binaries](https://github.com/JKGDevs/JediKnightGalaxies/releases/) , [Assets](https://icedrive.net/s/P35WiVwjAtT6Rt3QA27ADhZwjDwY) , 
[Map Bundle #1](http://jkhub.org/files/file/2652-jedi-knight-galaxies-map-bundle-1/) , [Map Bundle #2](https://jkhub.org/files/file/3778-jedi-knight-galaxies-map-bundle-2/)



## Installation of Game ##
JKG is available for Windows 11+ and Linux.  It will compile with most Linux distributions and MacOS; however it is only tested on Mint/Ubuntu and Windows.  We do not provide mac builds at this time.

To install, you will first need Jedi Academy installed. If you don't already own the game you can buy it from online stores such as [Steam](http://store.steampowered.com/app/6020/), [Amazon](http://www.amazon.com/Star-Wars-Jedi-Knight-Academy-Pc/dp/B0000A2MCN), or [GOG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy).

There are a few ways to actually install JKG itself, the simplest is to just get the [binaries](https://github.com/JKGDevs/JediKnightGalaxies/releases) and [assets](https://icedrive.net/s/P35WiVwjAtT6Rt3QA27ADhZwjDwY), and then follow the directions in the readme included.  Please note that while we try to keep these relatively up to date, the github will always contain the most current version of the source code and you can always compile and pack the pk3s yourself to get a more up to date version.  Most of the JKG assets are not stored on this github repository, only source code and code-like assets.

Alternatively you can install the assets and binaries inside a directory such as C:\JKG\ and then use a batch file/command line arguments to open jkgalaxies.x86.exe while pointing it to the Jedi Academy assets.  This is the recommended setup for developers, as you can then call your exe's location with appropriate args directly from your IDE.  You can do this by setting the fs_basepath arg to be equal to your Jedi Academy install location.  Optionally you can use fs_homepath to store screenshots and other user specific information, if not specified fs_homepath will be set to "...Documents\My Games\JKGalaxies"  For example, use this batch script on steam installs for windows:

	jkgalaxies.x86.exe +set fs_basepath "C:\Program Files (x86)\Steam\SteamApps\common\Jedi Academy\GameData" +set r_mode -2

If you're stuck, you can also watch [this video](https://www.youtube.com/watch?v=odx-3f07_eA) to see how to install the game.


## Dedicated Server ##
In order to run or host a dedicated JKG server, use the JKGalaxiesDed binary with the following args: `jkgalaxiesded.x86.exe +set fs_basepath ".." +set dedicated 1 +exec server.cfg`. Running dedicated from the main executable is currently not possible because it was intentionally broken with the addition of modular renderer.  A more detailed tutorial for setting up a Phase 1 server can be found [here](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Server-Guide-(Phase-I)).


## Current Project Goals ##
These are subject to change, especially phases later than Versus.  The idea behind the phase system is that by releasing the game into smaller chunks, we can more quickly receive feedback, test it, and continously build upon previous work.  Each phrase brings new systems and functionality to the game, as well as updating the game's core design and gameplay loop in significant ways.  We plan on having a final "legacy" version of each phase available once it is complete, in addition to the current version of the game.

### Phase 1:  Versus (current iteration). ###
* The goal here is to have a functioning arena based shooter (similar to games such as Call of Duty or Unreal Tournament) with the beginnings of some later RPG elements such as armor, pazaak card game, etc.  There are three major milestones currently being worked on: [Milestone 3](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=issue+milestone%3A%22Versus+Revision+3%22) and [Milestone 4](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=is%3Aopen+is%3Aissue+milestone%3A%22Versus+Revision+4%22).  Milestone 3 includes new features like better melee, armor, shields, jetpacks, debuff system etc. in addition to some bug fixes not addressed in Milestone 2. Milestone 4 primarily focuses on the all new saber system and related features such as duel mode.  Later milestones will introduce the skill trees and a rudimentary system for spending xp on skills in preparation for the eventual (classless, point based) leveling system that be implemented in greater detail in Phase 2 and 3.  The Force will be reworked and greatly expanded on using the aforementioned skill tree system. 

#### Phase 2: Coop ####
* Gameplay will primarily consist of teams of players (or individuals) completing simple 'quest' objectives, while being opposed by NPCs, or (in rare cases) other teams of players. The game will revolve around a major central city hub (likely Mos Eisley) from which player's can interact with NPCs and other players to start quests, form teams, and explore the world. Most quests that do not take part in the central city, will load a team of player's into an 'instance' where they will complete their quest objectives while isolated from other players not in their group. Player's will be able to enter conversations with various NPCs, make dialogue choices, try to solve NPC character's problems (or take advantage of them) and fight against them in PVE content as part of cooperative team of players.  The main features that are added include the NPC system and dialogue system. NPC AI will be able to wield JKG weapons and use them strategically along with a scriptable LUA system for managing and directing NPCs. Phase 2 also include overhauls to [player trading](https://jkhub.org/topic/8657-conceptplayer-trading/)</a>, [Looting/Notoriety System](https://jkhub.org/forums/topic/9879-conceptpvp-looting-wip/), chat systems, [player social systems](https://jkhub.org/topic/10889-conceptplayer-organizations/), fonts, UI, dynamic music system, a new renderer (rend2) and more. Additional fleshing out of the leveling system and skill trees will also occur.   We also plan to add some mechanics for [resource gathering and crafting mechanics](https://jkhub.org/forums/topic/11588-conceptcrafting/) during this phase. We may also attempt some MoBA like features (still needs discussion) in larger Phase 1 style matches.  Data is not persistent and lasts only for the duration of the server's play session.

#### Phase 3: RPG ####
* Open World RPG. The main features added here include quests, (more) minigames, and AI for existing minigames, like Pazaak. Also includes a more in depth leveling, XP and skill tree/point buying system, single/multiplayer player quests, large planet-themed maps (these will use chunk/cell loading), a robust character creator, and build upon several earlier mechanics. This mode will mainly focus on implementing things from a single player's perspective (within a multiplayer environment) and polishing previous mechanics. One major change from Phase 2 is that data will be semi-persistent and be tied to individual servers.

#### Phase 4: LMO ####
* FFeatures to the main game include larger group and faction quests, Looking For Group system, and a lot of the dungeons that were designed for Phase 2 brought back. Includes a master server based architecture and functioning account system. Data is stored persistently by a master server and shared between participating host servers, and official servers are all interconnected.  (You can still run private Phase 3 style servers if you want to).  Other necessary Large Multiplayer Online mechanics to facilitate large amounts of players will be added.  At this point, the game is pretty much done.  Community content updates will likely be the focus of future development efforts.
* Later possible expansions: space vehicles and battles, more worlds, procedural content, new quests, additional minigames etc.


## How can I help? ##
If you want to help contribute to JKG there's a lot of ways you can do so, here are some examples:
* Coding: See [below](https://github.com/JKGDevs/JediKnightGalaxies/#if-you-wish-to-contribute-code-to-jkgalaxies-please-do-the-following).  
* Participate: Joining the [Discord](https://discord.gg/YuG8Zks) and chatting with the developers or community in the #development channel is the most important way to get involved.  You can also find other players here to schedule matches with.
* Local Play: Host a LAN party with JKG as a featured game.  While JKG works on the internet, it can also be played with your local friends and is a great way to introduce people to the game!
* Streaming and Recording: Recording gameplay footage or streaming yourself playing JKG is another great way to introduce people to the game.
* Get the word out: Tell your friends about JKG, invite them to join scrimmage matches - the more the merrier.  Star this repo, the visibility helps!
* Server Hosting: Host a JKG game server (see [hosting guide here](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Server-Guide-(Phase-I))).
* Modelling, Mapping etc: Join the [Discord](https://discord.gg/YuG8Zks), and ask in the #development channel if you can help contribute.  Providing us examples of your work is appreciated and we can usually suggest things that we need help with.  One of the best ways to show you can help is to mod JKG and show off the results (ie: make a new weapon or new map for the game).  A video is a good way to showcase something like this.
* Audio/Music: Offer to help in the Discord and post samples of your work, the developers would love to put your talent to good use!


## Developer Notes ##
JKG is licensed under GPLv2 as free software. You are free to use, modify and redistribute JKG following the terms in [LICENSE.txt](https://github.com/JKGDevs/JediKnightGalaxies/blob/master/LICENSE.txt).
Please be aware of the implications of the GPLv2 licence. In short, be prepared to share your code under the same GPLv2 licence.  

### Build Guide ###
Want to build and compile JKG yourself from the source code? Checkout these guides:  
[JKG Compilation Guide](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Compilation-Guide)  
[Flate's Ubuntu/Debian Step-By-Step Guide](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Flate%27s-Debian-Ubuntu-Compiling-Guide)

### Dependencies ###
* A C++14 Compliant Compiler such as [Microsoft Visual Studios](https://visualstudio.microsoft.com/vs/community/) or [GCC](https://gcc.gnu.org/)
* [CMake (3.12.0+)](https://cmake.org/download/) (for build system)
* [Git (2.50.0+)](https://git-scm.com/download/) (for build system)
* SDL2 (2.30.8+) (included on Windows)
* OpenGL
* OpenAL (included on Windows)
* libpng (included on Windows)
* libjpeg (included on Windows)
* zlib (included on Windows)

### If you wish to contribute code to JKGalaxies, please do the following ###
* First, compile the project and get it running on your own machine first (as outlined [in the Compilation Guide here](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Compilation-Guide))
* Find [an issue](https://github.com/JKGDevs/JediKnightGalaxies/issues) to work on.
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github.
* Switch to the `develop` branch on your local copy of your fork. (`git checkout develop`)
* Create a new branch based on develop and make your changes.  Please note that master branch represents the current stable release, while the develop branch represents new changes that will be made public during the next release, but have not yet been finished.  It is usually best to start new changes by creating a new branch based on `develop`.  It's also usually a good idea to check with the developers on [Discord](https://discord.gg/YuG8Zks) before working on something new.  The #development channel on the Discord is the best place to ask questions about code, get help, or show off things that you are working on.
* It is often a good idea to try something simple first, such as modifying an existing feature in order to learn how JKG works, before trying to create brand new features.  The `ext_data` directory is a good place to start.
* Once you have finished your new feature and tested it, merge it back into your fork's develop branch.
* Send a [pull request](https://help.github.com/articles/creating-a-pull-request) to upstream (JKGDevs/JediKnightGalaxies).  We will review your request and then approve it if it looks good.  Please note that it is helpful to specify how you would like to be credited in your pull request.  Also make sure that you have permission to use everything you have included in your pull request.  (For example, make sure your code is GPLv2 compliant).
* Celebrate!  We appreciate every contribution!

#### A brief note about generative AI/LLM usage ####
TLDR: Don't.

We don't want to see it spamming our repo.  Why?  While we have nothing against our community or individuals personally choosing to use AI to assist, learn, or develop their own software, it does not belong in the repo.  Jedi Knight Galaxies is at its heart a hobby project, and a work of art.  We work on it because it is something we enjoy, and therefore the experience and journey of making JKG matters more than the end result.  Additionally, machines neither understand nuance, taste, opinion, security; nor can they be trusted with final decisions.  While it is fine to use it to help you find and fix bugs, understand the codebase, or write boiler plate outlines – we do not trust it to be competent enough to correctly diagnose issues, nor understand the team's vision well enough to be trusted with something as complex as creating a pull request.  

What does this mean for a contributor?  It means that you don't submit pull requests that were written mostly by or entirely by AI agents/tools.  You need to understand what your pull request is doing, why it is needed and why your fix solves the issue.  You need to back up your claims and test it yourself (that doesn't just mean unit tests! It means booting the game up, and testing extensively making sure your feature/fix performs as intended without causing other side effects or impacting performance in a bad way).  If you are trying to fix an issue/bug and use AI to help you, don't just trust it to be correct!  Do the work yourself, look at the proposed solution, try it, prove it, test it and make sure that it  makes sense...then after doing all that due diligence you can start writing up a pull request, yourself.  It takes time to understand the game and the code – especially to the point where you can explain in a pull request why you are wanting to change something.  **If you submit multiple successive pull requests, open multiple issues – you better be doing it of your own volition and understanding.  If there is evidence or patterns consistent with using AI tools to generate these, we will treat it like it is: spam.   This may result in a ban or other corrective action to protect the project and our hobby.**  Furthermore, please consider that because most AI's are not trained on only GPLv2 compliant code, they may regurgitate code or content that may be copyrighted or might not be compliant with the GPLv2 license.  If you’re not sure whether AI use may be an issue, please error on the side of caution and ask us first.

"If droids could think, there'd be none of us here." *-Obi-Wan*  

### If you wish to base your work off JKGalaxies (mod or engine) ###
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github
* Change the GAMEVERSION define in codemp/game/g_local.h from "jkgalaxies" to your project name
* If you make a nice change, please consider back-porting to upstream via pull request as described above. This is so everyone benefits without having to reinvent the wheel for every project.  The developers are usually more than willing to collaborate with other projects for everyone's benefits.
* While it is not a requirement to notify us of new projects based on JKG, you must follow the rules of the GPLv2 Licence (see above).  Typically this means making your source code available to the public.  We also enjoy checking out cool new projects based on JKG, so we'd love to hear about what you're doing with it.

Please use discretion when making issue requests on GitHub. The [forum](https://jkhub.org/forum/117-jedi-knight-galaxies/) is a better place for larger discussions on changes that aren't actually bugs, especially in relation to new ideas for features/gameplay.  If you're unsure of how something works or need clarification, it is best to ask the developers on the [Discord](https://discord.gg/YuG8Zks) in the #development channel.


## Version Information ##
JKG uses the following version schema: Phase.Major.Minor with an optional suffix (or "patch")letter following the minor version for hotfixes (these are unplanned versions that address server side only fixes and do not require client updates to play).  For example, the current version of the game is v`1.3.26`.  Phases represent collosal changes to the game (these are often called 'expansions' in other games) that include major new features and gameplay changes and even engine changes!  Phases should be considered seperate games.  Major versions represent completions of milestones that include several key new features and bug fixes.  Minor versions represent small incremental changes within a milestone and usually represent a single new feature or small set of features and/or bug fixes.  Other software produced by the developers (such as the launcher) uses its own versioning scheme and is not covered in this readme.  The version information for the game is defined in `codemp/qcommon/game_version.h`.


## Repository Organization ##
The repo is organized into the following directories.  Not all subdirectories are shown, just those of most significance.

* `root`: (You are here).  Contains tools for automated builds and cmake.  Also includes documentation such as this readme, `LICENSE.txt`, or the `Extended Data.txt` documentation on JSON style data the game uses.  Also includes batch files for generating Microsoft Visual Studio solution projects.
* `CMakeModules`: Contains files needed for configuring CMake builds so you can easily use make to build the project or generate project solutions.
* `JKGalaxies`: Contains json data files and other configuration settings used by the game (such as .itm files which define items in JKG).  Also contains the games lua scripts.  The contents of this directory are packed into a .pk3 file (zip) named `zz_JKG_Assets5.pk3` when packed for release.  Contains the following sub directories:
  - `ext_data`: Contains most of the game's json style formatted data.  Most people will want to start by playing with the files here to get a good idea of how to create new content for JKG.  Changing the stuff here will not require a new binary compilation and is intended to be human readable so as to make adjusting settings and game content easy for anyone including non-coders.  For example you can go to `ext_data/weapons/carbine_0_E-11_Carbine.wpn` to adjust the E-11 carbine's stats and settings.
  - `glua`: The game's lua scripts.  Can be hotloaded by reloading the current map with a map_restart.
  - `models`: Model information such as `_humanoid/animation.cfg` used to define things like animations.
  - `shaders`: Contains the game's shaders.
  - `strings`: Contains strings used by the game such as `strings/English/jkg_items.str` contains English translations for placeholder text.
  - `ui`: UI code (mostly menus).
* `JKGServer`: Contains additional configuration information needed to run a JKG server.  For example, the account list `accountlist.json` or server configuration settings `server.cfg`.  The contents of this directory are placed in the JKG subdirectory of the game when packed for release.
* `_Deprecated GLUA`: Contains Lua code no longer used by the game, but may be useful as examples of how to use lua to interact with the game or that may eventually be added back in.  Be warned most of these contain errors as they have not been updated to work with the current GLUA system. Many of these are quite old.
* `codemp`: The game's source code.  Contains the following subdirectories:
  - `botlib`: For handling multiplayer bots (mimic player behavior).  Mostly unchanged from JKA.
  - `cgame`: Contains cg code (eg: user interface code such as cg_scoreboard.cpp or cg_view.cpp).  For drawing mainly 2d things on the client's screen.
  - `client`: Contains client code (eg: handling key input, networking client to talk to server, sound, etc)
  - `game`: Contains a large portion of the game's game logic, such as g_combat.cpp which handles how combat/damage works between players/npcs.  
  - `ghoul2`: Handles ghoul2 model format system.
  - `GLua`: Hooks lua scripts into the game and provides an interface for lua to interact with the game. eg: glua_player.cpp has functions that can be used on/by player entities.
  - `icarus`: Old scripting system mostly leftover from JKA, no longer really used.
  - `libraries`: 3rd party libraries, such as jpeg formating (jpeg-9a), openssl, etc.
  - `macosx`: Contains info for building for JKG's mac .app and making renderers compile on osx.
  - `mp3code`: For handling mp3s.
  - `null`: Stub functions for dedicated servers.
  - `qcommon`: For handling generalized functions that need mostly global access, such as math stuff, or random number generators, etc.
  - `Ratl`: Used by raven for various standard templates.
  - `Ravl`: Raven's library for handling vectors.
  - `rd-common`: Shared common libraries used by the renderers (such as font code).
  - `rd-dedicated`: Dedicated render code.
  - `rd-vanilla`: The vanilla renderer code.
  - `server`: Handles serve side code logic (for talking to clients).
  - `ui`: Handles ui code, such as jkg_shop.cpp which draws the shop interface on the screen and allows you to interact with it.
  - `win32`: Code needed for windows build.
  - `zlib`: zlib library.
* `shared`: 3rd party source code such as sdl or system headers and binary icons.
* `tools`: Small script examples useful for helping make a developer's job easier.


## Contributors ##
This project is based on [OpenJK](https://github.com/JACoders/OpenJK) , which in turn is based on the source release of Raven's Jedi Academy (which was based on Quake3).  

### Active Developers ###
* Futuza
* Noodle
* Silverfang

 #### [JKG Contributors and Credits](https://www.jkgalaxies.net/contributors.html) ####
Over 100 people have contributed to, or worked on Jedi Knight Galaxies.  We are humbled so many have taken interest in the project or allowed us to use their work.  Thank you all who have contributed in any way!

JEDI KNIGHT GALAXIES AND ITS CONTENTS ARE NOT MADE, DISTRIBUTED, OR SUPPORTED BY ACTIVISION PUBLISHING, INC., RAVEN SOFTWARE, OR LUCASARTS ENTERTAINMENT COMPANY, LLC. ELEMENTS™ & © LUCASFILM LTD.™ & DISNEY, INC.™ AND/OR ITS LICENSORS. STAR WARS®, JEDI®, & JEDI KNIGHT® ARE REGISTERED TRADEMARKS OF LUCASFILM LTD™ AND WALT DISNEY, INC.™ STAR WARS®, JEDI®, & JEDI KNIGHT® ARE REGISTERED TRADEMARKS OF LUCASFILM LTD™ & DISNEY, INC.™  ALL LOGOS, IMAGES AND ANY OTHER TYPES OF MATERIALS ARE THE PROPERTY OF THEIR RESPECTFUL OWNERS.
  
