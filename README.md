# Replit3DS - Nintendo 3DS IDE

A complete integrated development environment (IDE) for creating Nintendo 3DS homebrew applications and games, running directly on your 3DS console!

## Features

- **Project Management**: Create, open, and manage 3DS homebrew projects
- **Code Viewer**: View source code files with scrolling on your 3DS (read-only)
- **Dependency Manager**: Track and manage libraries and dependencies
- **Build Instructions**: Step-by-step guidance for compiling projects on PC
- **Home Menu Integration**: Instructions for installing built apps to 3DS home menu
- **File Browser**: Navigate and manage project files on SD card
- **Project Templates**: Quick start with pre-configured C++ project templates

**Note**: Due to 3DS hardware constraints (limited RAM, no native keyboard), full text editing is not implemented on-device. Edit source files on your PC using your preferred IDE, then sync back to the 3DS SD card. This app focuses on project organization, code viewing, and providing build/install guidance.

## Building

### Requirements

- devkitPro (devkitARM)
- libctru
- citro3d and citro2d
- 3DS development tools (tex3ds, bannertool, makerom, 3dsxtool)

### Installation

```bash
# Install devkitPro from https://devkitpro.org/wiki/Getting_Started

# On Debian/Ubuntu-based systems:
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
sudo ./install-devkitpro-pacman

# Install 3DS development tools
sudo dkp-pacman -S 3ds-dev

# Install required libraries
sudo dkp-pacman -S libctru citro3d citro2d
```

### Compile

```bash
make
```

This will create `Replit3DS.3dsx` which can be run via the Homebrew Launcher or Citra emulator.

### Create CIA for Home Menu

To create a CIA file that can be installed to the home menu:

```bash
# First create a banner
bannertool makebanner -i banner.png -a audio.wav -o banner.bnr

# Create icon (if not auto-generated)
bannertool makesmdh -s "Replit3DS" -l "Nintendo 3DS IDE" -p "Replit" -i icon.png -o icon.smdh

# Build CIA
makerom -f cia -o Replit3DS.cia -elf Replit3DS.elf -rsf Replit3DS.rsf -icon icon.smdh -banner banner.bnr -exefslogo -target t
```

## Usage

### On 3DS Hardware

1. Copy `Replit3DS.3dsx` to `/3ds/` folder on your SD card
2. Launch via Homebrew Launcher
3. Use D-Pad or touch screen to navigate menus

### In Citra Emulator

1. Open Citra
2. File → Load File → Select `Replit3DS.3dsx`
3. Use keyboard or controller to navigate

## Controls

- **D-Pad**: Navigate menus and interface
- **A Button**: Confirm selection
- **B Button**: Back/Cancel
- **Touch Screen**: Direct interaction with UI elements
- **START**: Exit application

## Creating Projects

1. Select "New Project" from main menu
2. Enter project name
3. Choose project template (C++, C, or Empty)
4. Project structure will be created in `/3ds/replit3ds_projects/`

## Project Structure

```
/3ds/replit3ds_projects/
  └── YourProject/
      ├── source/         # Source code files
      ├── include/        # Header files
      ├── data/           # Data files
      ├── gfx/            # Graphics files
      ├── Makefile        # Build configuration
      └── build/          # Compiled output
```

## Managing Dependencies

The IDE can install common 3DS libraries:

- **libctru**: Core 3DS library
- **citro3d**: 3D graphics
- **citro2d**: 2D graphics
- **sf2d**: Simple and Fast 2D library
- **sftd**: Font rendering
- **sfil**: Image loading

Dependencies are stored in `/3ds/replit3ds_deps/`

## Building Projects

1. Open a project
2. Select "Build Project"
3. Output files will be created in project's `build/` directory
4. `.3dsx` file can be run via Homebrew Launcher
5. `.cia` file can be installed to home menu

## Installing to Home Menu

1. Build your project (creates `.cia` file)
2. Select "Install to Home Menu"
3. Application will appear on your 3DS home menu
4. Requires custom firmware (CFW) with FBI or similar installer

## Icon Requirements

To add a custom icon to your 3DS application:

1. Create a 48x48 pixel PNG image
2. Save as `icon.png` in project root
3. Icon will be automatically embedded during build

## License

This project is open source. Nintendo 3DS is a trademark of Nintendo Co., Ltd.

## Credits

- Built with devkitPro toolchain
- Uses libctru, citro3d, and citro2d libraries
- Developed for the Nintendo 3DS homebrew community

## Support

For issues, questions, or contributions, visit the project repository.

---

**Note**: This is homebrew software. Requires a 3DS with Homebrew Launcher access. Custom firmware (CFW) is required for CIA installation to home menu.
