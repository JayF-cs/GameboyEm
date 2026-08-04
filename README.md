# Game Boy Emulator

A Game Boy emulator written in C++, with SDL2 for rendering.

## Features

- Full CPU (Sharp LR35902) instruction set implementation
- PPU (picture processing unit) for graphics rendering
- Cartridge support for MBC1–MBC5 mapper types
- Timer and interrupt handling
- Joypad input
- Battery-backed save RAM (`.sav` file save/load), including RTC
  serialization for MBC3 cartridges with a real-time clock
- SDL2-based rendering

### Not yet implemented

- Audio (APU) — planned
- Game Boy Color (CGB) support — planned

## Requirements

- [SDL2](https://www.libsdl.org/)
- CMake 3.10+
- A Game Boy boot ROM — this project uses the open-source
  [Bootix](https://github.com/Hacktix/Bootix) boot ROM. You'll need to
  supply this file yourself; it is not included in this repository.
  By default the emulator looks for it at `../ROMS/bootix_dmg.bin`
  (relative to the working directory you run the executable from), but
  you can point it at a different file — see [Usage](#usage) below.

### Windows (MSYS2 / MinGW)

The provided build script assumes an MSYS2 UCRT64 toolchain:

- [MSYS2](https://www.msys2.org/), with the UCRT64 environment installed
- `g++` and `mingw32-make` available under `C:/msys64/ucrt64/bin/`
  (install via `pacman -S mingw-w64-ucrt-x86_64-gcc` and
  `mingw-w64-ucrt-x86_64-make` in the MSYS2 shell)
- `SDL2` available to that toolchain (`pacman -S mingw-w64-ucrt-x86_64-SDL2`)

## Building

```bash
./build.sh
```

This script wipes and recreates the `build/` directory, configures the
project with CMake using the MinGW Makefiles generator, and builds it
with `mingw32-make`. The resulting executable is `EMULATOR.exe`
inside the `build/` directory.

> If your MSYS2 installation isn't at `C:/msys64`, edit the compiler
> and make-program paths in `build.sh` to match your setup.

## Usage

```bash
EMULATOR.exe <rom_path.gb> [boot_rom_path.bin]
```

- `rom_path.gb` — path to the Game Boy ROM to run (required).
- `boot_rom_path.bin` — path to the boot ROM (optional). If omitted,
  defaults to `../ROMS/bootix_dmg.bin`.

If no ROM path is given, the emulator prints a usage message and exits.

## Controls

Controls are currently hardcoded:

| Game Boy button  | Key       |
|------------------|-----------|
| D-Pad Up         | Up Arrow  |
| D-Pad Down       | Down Arrow|
| D-Pad Left       | Left Arrow|
| D-Pad Right      | Right Arrow|
| A                | Z         |
| B                | X         |
| Select           | Left Shift|
| Start            | Enter     |
| Quit             | Esc       |

## Save Files

Cartridges with battery-backed RAM save to a `.sav` file. For MBC3
cartridges with a real-time clock, RTC state is serialized alongside
the save data. Saves are loaded on ROM load and written out on quit.

## Known Issues

- **Pokémon Yellow — freeze on Pikachu intro animation:** if no input
  is given before the intro animation finishes, the screen goes blank
  and the emulator appears to hang. Providing input during the
  animation avoids the issue entirely. The CPU is confirmed to still
  be executing instructions (not a `HALT` deadlock), and interrupts
  are still being serviced — the freeze is due to some polled
  condition never getting satisfied, currently under investigation.

## Roadmap

- Audio (APU) emulation
- Game Boy Color (CGB) support

## Contributing

Bug reports, issues, and pull requests are welcome — if you spot a bug,
have an improvement, or want to help with the known issues or roadmap
items above, feel free to open an issue or PR.

## License

TBD.