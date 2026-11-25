# My vkQuake fork

This GitHub repository is my vkQuake fork. The main purpose of my code changes is to add features that allow statistics for each match played to be exported to JSON files and to associate a Quake demo file with each one.

## Statistics

This section describes my specifications regarding the statistics features.

### Generated JSON Structures

I won't explain each JSON field, I consider their names to be self-explanatory. The example JSON object below represents a level that has been played.

```json
{
  // These numeric value is manage by quake
  // it starts incrementing when the you spawn and stop when the statistics
  // recording stop.
  "killed_monsters": 6,
  "total_monsters": 10,
  // Same for this.
  "found_secrets": 0,
  "total_secrets": 6,
  "mapname": "e1m1",
  "levelname": "the Slipgate Complex",
  "gamename": "id1",
  "map_time": 47.000000,
  "current_skill": 0,
  "demoname": "fdf8b8ff-4532-4352-81b2-594afb3d8395.dem",
  // Unix timestamp
  "timestamp": 1795465472,
  "engine_name_and_ver": "vkQuake 1.37.1",
  "statistics_ver": "1.0",
  "is_completed": true,
  // True if the statistics recording start and stop is automatic, it
  // will only happens it cl_statistics_auto is set to 1.
  "is_automatically_produced": true,
  "deaths": 2,
  // These counters are set to 0 when the statistics start recording.
  "inputs_counters": {
    "jump": 74,
    "forward": 60,
    "moveleft": 69,
    "moveright": 72,
    "back": 10,
    "attack": 32
  }
}
```

A list of items like the one above represents the levels played on a single map, such as "e1m1".

### Generated Files

Once the statistics have been generated, the JSON data is written to files, with each file representing a single map. The names of these files follow the format `statistics_<mapname>.json`. Additionally, a Quake demo file will be created and moved to the same folder as the JSON file. All generated files are placed in a folder whose path is specified by the command-line option `-statisticsdir`.

Only vkQuake should write and edit this files.

### Quake console commands

The table below summarizes the Quake console commands specifications.

| Name | Description |
| - | - |
| `statistics_start` | Starts recording statistics and the demo. |
| `statistics_stop` | Stops recording statistics and the demo. Writes the JSON file to the destination folder and then moves the Quake demo file to the destination folder. |
| `statistics_cancel` | Cancels the recording of statistics and the demo. Deletes the Quake demo file. |

### Quake console variable

The table below summarizes the Quake console variables specifications.

| Name | Description | Type |
| - | - | - |
| `cl_statistics_auto` | If enabled, recording of statistics and the demo will start automatically at the beginning of each level. They will stop automatically once the level is completed. | Boolean |

### Meson options

The table below summarizes the Meson options.

| Name | Description | Type |
| - | - | - |
| `statistics_debug` | Enable statistics debug features, including Quake console commands. | Boolean |

### Recommended usage

Personally, I find that the most convenient and simple approach is to use a bind like the one below.

```text
bind c toggle cl_statistics_auto 
```

## Installation

Windows and Linux binaries can be found in [Releases](https://github.com/Novum/vkQuake/releases).
MacOS (both Apple Silicon and 64-bit Intel) binaries are at [Mac Source Ports](https://www.macsourceports.com/game/quake).

### Windows
It is recommended to use the installer on Windows. This sets up start menu entries for the classic and remastered Quake versions. The engine finds Steam, GOG and Epic Games Store installs automatically; otherwise it asks for the game folder once and remembers it. Save data and config files will be written to the user folder (`%APPDATA%\vkQuake`) instead of the Quake data folder.

Otherwise copy all files inside the `vkquake-<version>_windows_x64.zip` (Intel) or `vkquake-<version>_windows_arm64.zip` (Arm64) folder in the zip to the Quake base directory. Overwrite any existing files. Afterward to run the game just execute `vkQuake.exe`.

### Linux
Download the Linux AppImage from [Releases](https://github.com/Novum/vkQuake/releases). Make it executable and run it:

```sh
chmod +x vkQuake-*.AppImage
./vkQuake-*.AppImage
```

> **Note**\
> Make sure all data files are lowercase, e.g. "id1", not "ID1" and "pak0.pak", not "PAK0.PAK". Some distributions of the game have upper case file names, e.g. from GOG.com.

### OpenBSD

[OpenBSD](https://openbsd.org) includes vkQuake in the standard package repositories since version [6.6](https://www.openbsd.org/66.html).

If you're running `OpenBSD 6.6` or greater, you can install the package with:

```console
$ pkg_add vkquake
```

### FreeBSD

[FreeBSD](https://freebsd.org) includes vkQuake in the standard port/package repositories since version [11.3](https://www.freebsd.org/releases/11.3R/announce).

If you're running `FreeBSD 11.3` or greater, you can install the package with:

```console
# pkg install vkquake
```

Alternatively, you can build vkQuake with FreeBSD's port collection:
```console
$ cd /usr/ports/games/vkquake
# make install
```

### Quake '2021 re-release'

vkQuake has support for playing the 2021 re-release content. Follow installation instructions as above but copy the files into the rerelease folder.

## Vulkan
vkQuake shows basic usage of the API. For example it demonstrates render passes & sub passes, pipeline barriers & synchronization, compute shaders, push & specialization constants, CPU/GPU parallelism and memory pooling.

## Endianness
vkQuake only supports **little-endian** systems. 
The reason is, all known existing Vulkan-capable systems in the wild are little-endian. Consequently, all big-endian support from QuakeSpasm has been effectively removed. 
vkQuake wouldn't even start on an big-endian system, outputting a fatal error.

## Building
> **Note**\
> For Windows, you will need at least Vulkan SDK version 1.4.321.1 or newer.
> 
> For Linux, you will need at least Vulkan SDK version 1.2.162 or newer. When building for Linux this is not always the case for the SDK provided by the distribution. Install the latest LunarG SDK if necessary.

### Windows

Clone the vkQuake repo from `https://github.com/Novum/vkQuake.git`

Prerequisites:

* [Git for Windows](https://github.com/git-for-windows/git/releases)
* A [Vulkan-capable GPU](https://en.wikipedia.org/wiki/Vulkan_(API)#Compatibility) with the appropriate drivers installed
* Install the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home). Log out and back in after installation to make sure environment variables are set.

#### Visual Studio

* Install [Visual Studio Community](https://www.visualstudio.com/products/free-developer-offers-vs) with Visual C++ component.

Open the Visual Studio solution, `Windows\VisualStudio\vkquake.sln`, select the desired configuration and platform, then
build the solution.

#### MinGW

Setup your [MinGW-w64](https://sourceforge.net/projects/mingw-w64/) environment, e.g. using [w64devkit](https://github.com/skeeto/w64devkit) or [MSYS2](https://www.msys2.org/).


Build 64 bit Intel vkQuake:

~~~
cd vkQuake/Quake
make -f Makefile.w64
~~~

Build 64 bit Arm vkQuake:

~~~
cd vkQuake/Quake
make -f Makefile.w64a
~~~

If you are on Linux and want to cross-compile for Windows, see the `build_cross_win??.sh` scripts.

#### Meson

With [Meson](https://mesonbuild.com/), [Ninja](https://ninja-build.org/) and a compiler installed (LLVM/Clang on PATH, or run `meson setup --vsenv build` for MSVC):

~~~
cd vkQuake
meson setup build && ninja -C build
~~~

### Linux

Make sure that both your GPU and your GPU driver support [Vulkan](https://en.wikipedia.org/wiki/Vulkan#Support_across_vendors).

To compile vkQuake, first install the build dependencies:

Ubuntu:
~~~
apt-get install git meson gcc glslang-tools spirv-tools libsdl3-dev libvulkan-dev libvorbis-dev libmpg123-dev libx11-xcb-dev
~~~

Arch Linux:
~~~
pacman -S git meson flac glibc libgl mpg123 libvorbis libx11 sdl3 vulkan-headers glslang spirv-tools
~~~

Fedora:
~~~
dnf install git meson gcc glslang spirv-tools vulkan-loader-devel SDL3-devel mpg123-devel libvorbis-devel flac-devel opusfile-devel
~~~

On distributions that do not ship SDL3 yet, install the SDL2 development package instead (e.g. `libsdl2-dev`); the build falls back to SDL2 automatically.

Then clone the vkQuake repo:

~~~
git clone https://github.com/Novum/vkQuake.git
~~~

Now go to the Quake directory and compile the executable:

~~~
cd vkQuake
meson build -Ddebug=true -Dstrip=false && ninja -C build
~~~

Meson prefers SDL3 and falls back to SDL2 if it is not installed; add `-Duse_sdl3=disabled` to force SDL2 (or `enabled` to require SDL3).

On macOS the build links MoltenVK directly. Add `-Duse_vulkan_loader=enabled` to link the Vulkan loader instead, so the driver can be chosen with `VK_DRIVER_FILES`.

> **Note**\
> The Meson version needs to be 1.3.0 or newer. For older distributions you can use make:
> ~~~
> cd vkQuake/Quake
> make -j
> ~~~
> Meson is the preferred way to build vkQuake because it automatically checks for out of date file dependencies, is faster and has better error reporting for missing dependencies.

> **Note**\
> vkQuake requires **SDL3** or, as a fallback for older distributions, at least **SDL2 2.0.6 with enabled Vulkan support**.

### MacOS

To compile vkQuake, first install the build dependencies with Homebrew:

~~~
brew install molten-vk vulkan-headers glslang spirv-tools sdl3 libvorbis flac opus opusfile flac mpg123 meson pkgconfig
~~~

Then clone the vkQuake repo:

~~~
git clone https://github.com/Novum/vkQuake.git
~~~

Now go to the Quake directory and compile the executable:

~~~
cd vkQuake
meson build -Ddebug=true -Dstrip=false && ninja -C build
~~~

Meson prefers SDL3 and falls back to SDL2 if it is not installed; add `-Duse_sdl3=disabled` to force SDL2 (or `enabled` to require SDL3).

> **Note**\
> The Meson version needs to be 1.3.0 or newer.

## Error reporting

`vkQuake` is not guaranteed to be free of runtime errors. In those rare cases, the game is either quit brutally with a `Quake Error` dialog, 
or by an `Host_Error` console message that only terminates the currently played level. 

In both cases some error context is provided that can be useful for developers. See [Error reporting](error_reporting_howto.md) for details.      

## Optional - Music / Soundtrack

> **Note**\
> This section only applies to older releases. For the 2021 re-release music will work out of the box.

The original Quake had a great soundtrack by Nine Inch Nails. Unfortunately, the Steam version does not come with the soundtrack files. The GOG-provided files need to be converted before they are ready for use. In general, you'll just need to move a "music" folder to the correct location within your vkQuake installation (.e.g `/usr/share/quake/id1/music`). Most Quake engines play nicest with soundtracks placed in the `id1/music` subfolder vs. `sound\cdtracks`

QuakeSpasm, the engine vkQuake is derived from, supports OGG, MP3, FLAC, and WAV audio formats. The Linux version of QuakeSpasm/VkQuake requires external libraries: libogg or libvorbis for OGG support, libmad or libmpg123 for MP3, and libflac for FLAC. If you already have a setup that works for the engine you're currently using, then you don't necessarily have to change it. 

Generally, the below setup works for multiple engines, including Quakespasm/vkQuake:

* The music files are loose files, NOT inside a pak or pk3 archive.
* The files are placed inside a "music" subfolder of the "id1" folder. For missionpack or mod soundtracks, the files are placed in a "music" subfolder of the appropriate game folder. So the original Quake soundtrack files go inside "id1\music", Mission Pack 1 soundtrack files go inside "hipnotic\music", and Mission Pack 2 soundtrack files go inside "rogue\music".
* The files are named in the pattern "tracknn", where "nn" is the CD track number that the file was ripped from. Since the soundtrack starts at the second CD track, MP3 soundtrack files are named "track02.mp3", "track03.mp3", etc. OGG soundtrack files are named "track02.ogg", "track03.ogg", etc. FLAC soundtrack files are named "track02.flac", "track03.flac", etc. WAV soundtrack files are named "track02.wav", "track03.wav", etc.

**See more:** [Quake Soundtrack Solutions (Steam Community)](http://steamcommunity.com/sharedfiles/filedetails/?id=119489135)
