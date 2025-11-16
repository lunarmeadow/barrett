# Barrett

A Rise of the Triad source port based on [ROTTEXPR](https://github.com/LTCHIPS/rottexpr) focused on QoL for players and developers, expanded compatibility, new features, and better support for modern hardware.

## Requirements

Libraries:

`SDL2.dll` (https://www.libsdl.org/download-2.0.php)

`SDL2_mixer.dll` (https://www.libsdl.org/projects/SDL_mixer/)

Game Files:

```
DEMO1_3.DMO
DEMO2_3.DMO
DEMO3_3.DMO
DEMO4_3.DMO
REMOTE1.RTS
```

And

```
DARKWAR.RTC
DARKWAR.RTL
DARKWAR.WAD
```

Or

```
HUNTBGIN.RTC
HUNTBGIN.RTL
HUNTBGIN.WAD
```

Place all those files in the same directory as the barrett executable.

## Building

To build the project, you will need to install devkitpro pacman.

Run `sudo dkp-pacman -S 3ds-dev 3ds-portlibs`

Next, you will need to manually build SDL2 and SDL_mixer.

SDL:

```sh
git clone --single-branch --branch SDL2 https://github.com/libsdl-org/SDL.git && cd SDL
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

SDL_mixer:

```sh
git clone --single-branch --branch SDL2 https://github.com/libsdl-org/SDL_mixer.git && cd SDL_mixer
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" -DCMAKE_BUILD_TYPE=Release -DSDL2MIXER_DEPS_SHARED=OFF -DSDL2MIXER_BUILD_SHARED_LIBS=OFF -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF -DSDL2MIXER_WAVPACK=OFF
cmake --build build -j
sudo cmake --install build
```

This project currently uses CMake. You can create a build target and then build with the commands

```sh
cmake -Bcmake-build-3ds -DCMAKE_TOOLCHAIN_FILE=DevkitArm3DS.cmake -DCMAKE_BUILD_TYPE=Release -S.
cmake --build cmake-build-3ds
```

For CMAKE_BUILD_TYPE, please refer to the [CMake docs](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).  
You could alternatively create different build targets for registered, shareware etc.

Please review the CMakeLists.txt file to customize your build options.

# License

Barrett is licensed under the terms of the GNU GPLv2 license.

Copyright (C) 1994-1995 Apogee Software, Ltd.  
Copyright (C) 2002-2015 icculus.org, GNU/Linux port  
Copyright (C) 2017-2018 Steven LeVesque  
Copyright (C) 2025 lunarmeadow (she/her)  
Copyright (C) 2025 erysdren (it/its)  

The full license text for the GPLv2 can be found within `COPYING` in the root directory of this project.  
The full license text for Rise of the Triad: Dark War can be found within `ROTTLICENSE` in the root directory of this project.

The license for [3ds-cmake](https://github.com/Xtansia/3ds-cmake) can be found in `LICENSES`.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
