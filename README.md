# Monitor Maker

<p align="center">
    <image src="assets/main-application-window.png" height="400" alt="Screenshot of the main application window, showing all of the controls for the utility">
</p>

Portal 2 Monitor Maker is an open-source utility built on [sourcepp](https://github.com/craftablescience/sourcepp) with a UI built with [FLTK](https://www.fltk.org) that allows quick and easy creation of custom props to be used in Portal 2 mapping.
You can import your images, select relevant options and export without the hassle of decompiling Valve's model formats.

# Features

- Multiple skins (max 31)
- Animated textures
    - Customisable FPS
- Animated texture playback
- Scrolling textures
    - Scroll direction and custom speed options
- Scanline
- Both monitor poses
- Customisable model name

# Building

Linux (Ubuntu):
```
sudo apt update && sudo apt install cmake build-essential git -y

cmake --preset linux-release
cmake --build build/linux-release

# Running
./build/linux-release/monitor-maker
```
Windows:
```
cmake --preset windows-release
cmake --build build/windows-release --config Release

# Running
./build/windows-release/Release/monitor-maker.exe
```

# Contributing
All help is very much appreciated. Whether it be just using the application, testing it on various setups, submitting issues/feature requests or opening pull requests, everyone is welcome to participate.

While there may not be many features that could be added, there still are some that might be nice to have at some point:
- Executable icon (Windows)
- Better coding standards
- Vactube scanner TV model support
- Various exporting options (VPK?)
- Improved preview showing the scanline and scrolling
- Basic texture creation for text on a background
- Proper parsing and writing of .mdl files

## Lisence

Licensed under the MIT license