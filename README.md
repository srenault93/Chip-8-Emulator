# CHIP-8 Emulator

A CHIP-8 interpreter written in C++ with an SDL2 front-end. It runs the classic
1970s CHIP-8 ROMs (Pong, Tetris, Space Invaders, and the rest of the public
catalogue) and renders them with a green-phosphor look, an on-screen keypad, and
a small status bar.

## What it does

- A fetch/decode/execute loop covering the full standard CHIP-8 opcode set
- 64×32 monochrome display, scaled 10× to a 640×320 window
- 16-key hex keypad you can drive from the keyboard or by clicking the on-screen pad
- Pause/resume, soft reset, and adjustable emulation speed (1 to 30 cycles per frame)
- 60 Hz delay and sound timers (the beep is currently a console placeholder)
- Status bar showing pause state, speed level, and key hints

## Controls

The CHIP-8 keypad maps to the left side of a QWERTY keyboard:

```
 CHIP-8 keypad        Keyboard
 1 2 3 C              1 2 3 4
 4 5 6 D              Q W E R
 7 8 9 E              A S D F
 A 0 B F              Z X C V
```

You can also click the keys on the on-screen pad with the mouse.

| Key      | Action                      |
|----------|-----------------------------|
| `P`      | Pause / resume              |
| `R`      | Reset and reload the ROM    |
| `+` / `-`| Increase / decrease speed   |
| `Esc`    | Quit                        |

## Building

You need a C++11 compiler and SDL2.

```bash
# Debian / Ubuntu
sudo apt install build-essential libsdl2-dev

# Fedora
sudo dnf install gcc-c++ SDL2-devel

# Arch
sudo pacman -S base-devel sdl2
```

Then build from the `src` directory:

```bash
cd src
make
```

This produces a `CHIP-8-EMULATOR` binary.

## Running

Pass a ROM file on the command line:

```bash
./CHIP-8-EMULATOR path/to/game.ch8
```

CHIP-8 ROMs are widely available online and are in the public domain. Drop one in
and go.

## Project layout

```
src/
  main.cpp      SDL2 window, rendering, input, and the main loop
  chip8.h/.cpp  CPU state, memory, timers, and the fetch/execute cycle
  opcodes.h/.cpp  Opcode decoding and execution
  makefile      Build rules
```

## How it works

The CHIP-8 is a virtual machine with 4 KB of memory, 16 8-bit registers
(`V0` to `VF`), a 16-bit index register, a program counter, and a 16-level call
stack. Programs load at address `0x200`; the built-in 4×5 font lives at `0x050`.
Each cycle reads a two-byte big-endian opcode, and `OpcodeHandler` decodes it by
its top nibble and dispatches to the right instruction. The display is a 64×32
bit array that sprite-draw instructions XOR into, which is also how collision
detection (the `VF` flag) works.

## Notes

This started as a learning project. Sound is stubbed out, so a beep currently
prints `[BEEP]` to the terminal instead of playing audio.

## References

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [How to write an emulator (CHIP-8 interpreter)](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)
