# Dungeons of Noudar
A first person 2.5D dungeon-crawler for (MS/Free)DOS for *Protected-Mode*, written in C++14(ish), fixed point math and test coverage (for its core). As the 1990s tradition dictates!

Sound options include:

* Adlib
* PC-Speaker
* OPL2LPT

![ ](screenshot1.png?raw=true)
![ ](screenshot2.png?raw=true)

## Building

From inside `DungeonsOfNoudar486`, run:

```
make
```

This will build both the DOS executable and the data package. If you want just the data package, run:

```
make data
```

There is also an internal SDL2 version, meant for testing and development, available thru *CMake* or `Makefile.SDL`. This version will *NOT* assemble the data package.

## System Requirements

The game has been successfully tested on a 486 SX-33Mhz, with 8MB RAM. There are reports of it running on a 386, but I can't guarantee it's smooth enough.

## Core (Curses version)

The core stil is a valid text-mode dungeon crawler with almost the game geometry and rules as this version. In fact, when I was developing that one, I was already preparing the levels to be 3D (hence the somewhat gratuitous tiles variety). Go to `noudar-core` for building it.

## Other versions

This game began its life as an Android VR game (Cardboard) and for a while it run just file like that. Eventually, the allure of building something for REAL DOS got me and those versions fell on the wayside. You can still build those from the ***V2*** branch.

That branch also contains ports for other platforms, like iOS, OSX, LibRetro (as a core) and a botched Win32 version. Historically, there was also a OpenGL (software rendered) version for DOS, but it required too much CPU power and didn't look that pretty.


