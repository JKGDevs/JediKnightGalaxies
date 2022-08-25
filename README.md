# Jedi Knight Galaxies #
[Webpage](https://www.jkgalaxies.net)  
[Discord](https://discord.gg/YuG8Zks)   

[FAQ](https://www.jkgalaxies.net/faq.html)  
[Forum]( https://jkhub.org/forum/117-jedi-knight-galaxies/) (hosted by JKHub)  
[Forum Announcements](https://jkhub.org/forum/119-jkgalaxies-news/)   
[YouTube](https://www.youtube.com/channel/UCnnAUSngUk8l3fycYMVXkKQ/featured)  
[Twitch](https://www.twitch.tv/jkgalaxies)  
[ModDB Page](http://www.moddb.com/mods/jkgalaxies) (not updated frequently)  
[Odysee.com](https://odysee.com/@JediKnightGalaxies:c?view=about) (not updated frequently)  


### Downloads ###
The most update links are available on the download page on the website, under the [test releases section](https://www.jkgalaxies.net/download.html#testdiv), but you can also use the following links which are occasionally updated:

[Binaries](https://github.com/JKGDevs/JediKnightGalaxies/releases/) , [Assets](https://uvu.box.com/shared/static/ctdly3jwnhjcji560sx1czfdgnahbtzi.zip) , 
[Map Bundle #1](http://jkhub.org/files/file/2652-jedi-knight-galaxies-map-bundle-1/) , [Map Bundle #2](https://uvu.box.com/shared/static/kb0wvyqtz0sarzs0kn1c8h03wrpjgb7c.zip)


### What is Jedi Knight Galaxies? ###
Jedi Knight Galaxies is a competitive and innovative multiplayer shooter, played out in the Star Wars universe. Currently a full conversion mod, the project was originally based on and modified from Raven Software’s [Jedi Knight: Jedi Academy](https://en.wikipedia.org/wiki/Star_Wars_Jedi_Knight:_Jedi_Academy). Jedi Knight Galaxies is open source and available completely free of charge; however, you will need the original game in order to play. (Jedi Academy is frequently available on [Steam](http://store.steampowered.com/app/6020/STAR_WARS_Jedi_Knight__Jedi_Academy/) and [GoG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy) during sales for less than $4). We aim to develop the project into a Large Multiplayer Online game, crossing traditional RPG (role playing game) elements with the fast paced excitement of an FPS (first person shooter).

The project will be released in phases, each building on to the previous one with more aspects of the final goal. The current Phase, JKG: Versus, was first released September 17th, 2012 as a public beta, showcasing the primary FPS elements of the game. Subsequent updates and versions have been produced since then. As development progresses, sandbox environments will be added, initial RPG elements will appear, and player vs. enemy combat will be implemented into the faction oriented gameplay. Improved dialog and cinematic features along with skills, powers, and more aspects that belong in a Role-Playing Game will be developed, set between the events of Episode IV: A New Hope and Episode III: Revenge of the Sith. Finally, the full-fledged JKG: LMO takes the experience to a much grander level.

The game is a much loved hobby project supported by its [community and developers](https://www.jkgalaxies.net/contributors.html) who work on it in their freetime, contribution is encouraged and newbies are welcome. Originally founded by a group of Jedi Academy modders led by Jake (now retired), the project has since been in and out of development since 2008. The current development team comes from a variety of backgrounds, countries, and age, united by our desire to make a great game.

Welcome to Jedi Knight Galaxies, where we break the limits!


### Trailer ###
[![JKG v1.3.22 Trailer](http://img.youtube.com/vi/X63qy7lONyo/0.jpg)](http://www.youtube.com/watch?v=X63qy7lONyo "JKG v1.3.22 Trailer")


## Build Guide ##
[OpenJK Compilation Guide](https://github.com/JACoders/OpenJK/wiki/Compilation-guide)  
[Flate's Ubuntu/Debian Guide](https://github.com/JKGDevs/JediKnightGalaxies/wiki/Flate%27s-Debian-Ubuntu-Compiling-Guide)

## Installation of Game ##
JKG is available for Windows, but it will compile with most Linux distributions and MacOS; however it is only briefly tested on Ubuntu.

To install, you will first need Jedi Academy installed. If you don't already own the game you can buy it from online stores such as [Steam](http://store.steampowered.com/app/6020/), [Amazon](http://www.amazon.com/Star-Wars-Jedi-Knight-Academy-Pc/dp/B0000A2MCN), or [GOG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy).

There are a few ways to actually install JKG itself, the simplest is to just get the [binaries](https://github.com/JKGDevs/JediKnightGalaxies/releases) and [assets](https://uvu.box.com/shared/static/ctdly3jwnhjcji560sx1czfdgnahbtzi.zip), and then follow the directions in the readme included.  Please note that while we try to keep these relatively up to date, the github will always contain the most current version of the source code and you can always compile and pack the pk3s yourself to get a more up to date version.  Most of the JKG assets are not stored on this github repository, only source code and code-like assets.

Alternatively you can install the assets and binaries inside a directory such as C:\JKG\ and then use a batch file/command line arguments to open jkgalaxies.x86.exe while pointing it to the Jedi Academy assets.  This is the recommended setup for developers, as you can then call your exe's location with appropriate args directly from your IDE.  You can do this by setting the fs_basepath arg to be equal to your Jedi Academy install location.  You'll also want to set the fs_cdpath to be equal to JKG's directory.  Optionally you can use fs_homepath to store screenshots and other user specific information, if not specified fs_homepath will be set to "...Documents\My Games\JKGalaxies"  For example, use this batch script on steam installs for windows:

	jkgalaxies.x86.exe +set fs_game "JKG" +set fs_cdpath "." +set fs_basepath "C:\Program Files (x86)\Steam\SteamApps\common\Jedi Academy\GameData" +set r_mode -2

If you're stuck, you can also watch [this video](https://www.youtube.com/watch?v=odx-3f07_eA) to see how to install the game.

## Dependencies ##

* SDL2 (2.0.14+) (included on Windows)
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
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github.
* Create a new branch and make your changes.  Please note that master branch represents the current stable release, while the develop branch represents new changes that will be made public during the next release.  It is usually best to start new changes by creating a new branch based on develop.  It's also usually a good idea to check with the developers on [Discord](https://discord.gg/YuG8Zks) before working on something new.
* Send a [pull request](https://help.github.com/articles/creating-a-pull-request) to upstream (JKGDevs/JediKnightGalaxies)

### If you wish to base your work off JKGalaxies (mod or engine) ###
* [Fork](https://github.com/JKGDevs/JediKnightGalaxies/fork) the project on Github
* Change the GAMEVERSION define in codemp/game/g_local.h from "jkgalaxies" to your project name
* If you make a nice change, please consider back-porting to upstream via pull request as described above. This is so everyone benefits without having to reinvent the wheel for every project.  The developers are usually more than willing to collaborate with other projects for everyone's benefits.
* While it is not a requirement to notify us of new projects based on JKG, you must follow the rules of the GPLv2 Licence (see above).  Typically this means making your source code available to the public.  We also enjoy checking out cool new projects based on JKG, so we'd love to hear about what you're doing with it.

Please use discretion when making issue requests on GitHub. The [forum](https://jkhub.org/forum/117-jedi-knight-galaxies/) is a better place for larger discussions on changes that aren't actually bugs.  If you're unsure of how something works or need clarification, it is best to ask the developers on the [Discord](https://discord.gg/YuG8Zks) in the #development channel.


## Current Project Goals ##
These are very much subject to change, especially phases later than Versus.

### Phase 1:  Versus (current iteration). ###
* The goal here is to have a functioning arena based shooter (similar to games such as Call of Duty or Unreal Tournament) with the beginnings of some later RPG elements such as armor, pazaak card game, etc.  There are three major milestones currently being worked on: [Milestone 3](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=issue+milestone%3A%22Versus+Revision+3%22) and [Milestone 4](https://github.com/JKGDevs/JediKnightGalaxies/issues?q=is%3Aopen+is%3Aissue+milestone%3A%22Versus+Revision+4%22).  Milestone 3 includes new features like better melee, armor, shields, jetpacks, debuff system etc. in addition to some bug fixes not addressed in Milestone 2. Milestone 4 primarily focuses on the all new saber system and related features such as duel mode.  Later on other milestones will be worked on.  In Milestone 5, the Force will be reworked and greatly expanded on.  Later possible milestones will introduce the skill trees and a rudimentary system for spending xp in preparation for the eventual leveling system in Phase 2 and 3.

#### Phase 2: Coop ####
* The main features that are added here include the NPC system and fleshing out of the dialogue system. They also include overhauls to the chat systems, fonts, UI, and more.  Prep work for the leveling system and quest system to be introduced here.  Gameplay will primarily consist of teams of players (or individuals) completing simple 'quest' objectives, while being opposed by NPCs, or other teams of players.  The game will revolve around a major central city hub (likely Mos Eisley) from which player's can interact with NPCs and other players to start quests, form teams, and explore the world.  Most quests that do not take part in the central city, will load a team of player's into an 'instance' where they will complete their quest objectives while isolated from other players.  We may also attempt some MoBA like features (still needs discussion) in larger Phase 1 style matches.

#### Phase 3: RPG ####
* Open World RPG. The main features added here include quests, (more) minigames, and AI for existing minigames, like Pazaak. Also includes levelling up, experience, single person quests, large planet-themed maps, character creator, fully fleshed out skill system etc. This mode will mainly focus on implementing things from a single player's perspective.  Data will be persistent and exist as long as the server is running.

#### Phase 4: LMO ####
* Features to the main game include group quests, Looking For Group system, and a lot of the dungeons that were designed for Phase 2 brought back. Includes a master server based architecture and functioning account system.  At this point, the game is pretty much done.
* Later possible expansions: space battles, more worlds, additional minigames


## Version Information ##
JKG uses the following version schema: Phase.Major.Minor with an optional suffix (or "patch")letter following the minor version for hotfixes (these are unplanned versions that address server side only fixes and do not require client updates to play).  For example, the current version of the game is v`1.3.23`.  Phases represent collosal changes to the game (these are often called 'expansions' in other games) that include major new features and gameplay changes and even engine changes!  Phases should be considered seperate games.  Major versions represent completions of milestones that include several key new features and bug fixes.  Minor versions represent small incremental changes within a milestone and usually represent a single new feature or small set of features and/or bug fixes.  Other software produced by the developers (such as the launcher) uses its own versioning scheme and is not covered in this readme.


## How can I help? ##
If you want to help contribute to JKG there's a lot of ways you can do so, here are some examples:
* Participate: Joining the [Discord](https://discord.gg/YuG8Zks) and chatting with the developers or community in the #development channel is the most important way to get involved.  You can also find other players here to schedule matches with.
* Get the word out: Tell your friends about JKG, invite them to join scrimmage matches - the more the merrier.
* Server Hosting: Host a JKG game server.
* Coding: Compile the project and get it running on your own machine first (as outlined in this readme), then take a look at the [issues](https://github.com/JKGDevs/JediKnightGalaxies/issues) list for one that interests you.  (The pinned issue for the next patch covers the most critical issues needed for the next patch).
* Modelling, Mapping etc: Join the [Discord](https://discord.gg/YuG8Zks), and ask in the #development channel if you can help contribute.  Providing us examples of your work is appreciated and we can usually suggest things that we need help with.  One of the best ways to show you can help is to mod JKG and show off the results (ie: make a new weapon or new map for the game).  A video is a good way to showcase something like this.
* Audio/Music: Offer to help in the Discord and post samples of your work, the developers would love to put your talent to good use!


## Repository Organization ##
The repo is organized into the following directories.  Not all subdirectories are shown, just those of most significance.

* `root`: (You are here).  Contains tools for automated builds and cmake.  Also includes documentation such as this readme, `LICENSE.txt`, or the `Extended Data.txt` documentation on JSON style data the game uses.  Also includes batch files for generating Microsoft visual studio solution projects.
* `CMakeModules`: Contains files needed for configuring CMake builds so you can easily use make to build the project or generate project solutions.
* `JKGalaxies`: Contains json data files and other configuration settings used by the game (such as .itm files which define items in JKG).  Also contains the games lua scripts.  The contents of this directory are packed into a .pk3 file (zip) named `zz_JKG_Assets5.pk3` when packed for release.  Contains the following sub directories:
  - `ext_data`: Contains most of the game's json style formatted data.  Most people will want to start by playing with the files here to get a good idea of how to create new content for JKG.  Changing the stuff here will not require a new binary compilation and is intended to be human readable so as to make adjusting settings and game content easy for anyone including non-coders.  For example you can go to `ext_data/weapons/carbine_0_E-11_Carbine.wpn` to adjust the E-11 carbine's stats and settings.
  - `glua`: The game's lua scripts.  Can be hotloaded by reloading the current map with a map_restart.
  - `models`: Model information such as `_humanoid/animation.cfg` used to define things like animations.
  - `shaders`: Contains the game's shaders.
  - `strings`: Contains strings used by the game such as `strings/English/jkg_items.str` contains English translations for placeholder text.
  - `ui`: UI code (mostly menus).
* `JKGServer`: Contains additional configuration information needed to run a JKG server.  For example, the account list `accountlist.json` or server configuration settings `server.cfg`.  The contents of this directory are placed in the JKG subdirectory of the game when packed for release.
* `_Deprecated GLUA`: Contains Lua code no longer used by the game, but may be useful as examples of how to use lua to interact with the game or that may eventually be added back in.  Be warned most of these contain errors as they have not been updated to work with the current GLUA system.
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
### Active Developers ###
* Darth Futuza
* Noodle
* Silverfang


### Retired Developers ###
#### Coding ####
* BobaFett
* DeathSpike
* Didz
* eezstreet
* Raz0r
* Stoiss
* UniqueOne
* Vek892
* Xycaleth

#### Mapping ####
* dvg94
* MaceCrusherMadunusus
* Pande
* Sato
* SJC
* Yzmo

#### Modeling ####
* Blastech
* CaptainCrazy
* DarthPhae
* DT
* Dusk
* IG64
* Tri
* Pande
* Psycho
* Resuru

#### Animations ####
* Hirman

#### Artists ####
* BlasTech
* HellKobra
* Pande
* Resuru
* Suibuku

#### Sound and EFX ####
* Blastech
* Sareth

#### Misc ####
* BlueCasket
* Caelum
* DarthLex
* HellKobra
* Fighter
* Jake (Project Founder)
* JohnGWolf
* Konradwerks
* Mart
* Sharpie
* TheDarkness

#### Special Thanks ####
Thank you to the following for your support of JKG and your efforts in contributing to it with your time in various different ways even though it may not have been through directly developing it.  We appreciate the encouragement, advertising, resources, and feedback you've provided:  




* Arkan (Mapping)
* Daggo (server list)
* Dalo Lorn
* Flate (jk2t.ddns.net mirror and build help)
* Hapslash (Stormtrooper Model)
* Inyri Forge (Instruments & KotOR Objects)
* John/GCJ (Linux Debugging/Server Hosting)
* Krattle (Chalmun’s Cantina)
* MountainDew
* Nightcrawler
* Obliviion
* Ori'Ramikad
* Orj (Mosquito Vibroblade)
* Plasma (Mapping)
* Smoo
* Szico VII (Nightfall Map)
* Tommy
* WizardMKBK (help with icons)
* Xel
* [And many more!](https://www.jkgalaxies.net/contributors.html)
 
 

This project is based on [OpenJK](https://github.com/JACoders/OpenJK) , which in turn is based on the source release of Raven's Jedi Academy.  Thank you all who have contributed to either project!   JEDI KNIGHT GALAXIES IS NOT MADE, DISTRIBUTED, OR SUPPORTED BY ACTIVISION, RAVEN, DISNEY, OR LUCASARTS, A DIVISION OF LUCASFILM ENTERTAINMENT COMPANY LTD. ELEMENTS™ & (©) LUCASARTS, A DIVISION OF LUCASFILM ENTERTAINMENT COMPANY LTD.
