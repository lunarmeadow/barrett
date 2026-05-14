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

To build the project, you'll need the SDL2.0 development libraries  
(https://www.libsdl.org/download-2.0.php) as well as the SDL_mixer developement  
libaries (https://www.libsdl.org/projects/SDL_mixer/, under Binary).

For Linux users, install the respective SDL2 dependencies through your distro's package manager.  
Barrett can be compiled with either GCC or Clang. On Windows, use something such as MinGW's GCC.

This project currently uses CMake. You can create a build target and then build with the commands

```sh
cmake -Bcmake-build-barrett -DCMAKE_BUILD_TYPE=Release -S.
cmake --build cmake-build-barrett
```

For CMAKE_BUILD_TYPE, please refer to the [CMake docs](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).  
You could alternatively create different build targets for registered, shareware etc.

Please review the CMakeLists.txt file to customize your build options.

# License

Barrett is licensed under the terms of the GNU GPLv2 license.

Copyright (C) 1994-1995 Apogee Software, Ltd.  
Copyright (C) 2002-2015 icculus.org, GNU/Linux port  
Copyright (C) 2017-2018 Steven LeVesque  
Copyright (C) 2025-2026 lunarmeadow (she/her)  
Copyright (C) 2025-2026 erysdren (it/its)  

The full license text for the GPLv2 can be found within `COPYING` in the root directory of this project.  
The full license text for Rise of the Triad: Dark War can be found within `ROTTLICENSE` in the root directory of this project.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
