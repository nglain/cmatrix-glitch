# CMatrix Glitch

Matrix rain effect with **glitch mode** and **waterfall text** display.

```
    ▓░▒▓█▒░▓▒█░▒▓░█▒▓░▒█▓░▒▓█░▒
    ░▒▓█░▒▓░█▒░▓▒     ░▒▓█░▒▓░█
    ▓░▒▓█▒░▓▒█░   H E L L O   ▒
    ▒▓█░▒▓░█▒░▓▒     ▓█░▒▓░█▒░▓
    ░▒▓█░▒▓░█▒░▓▒█░▒▓░▒█▓░▒▓█░▒
```

Fork of [abishekvashok/cmatrix](https://github.com/abishekvashok/cmatrix) with extended features.

## Features

### Glitch Mode (`-G`)
Replaces matrix symbols with gradient blocks for a rain/waterfall visual effect:
```
█ ▓ ▒ ░ (bright → dim)
```
Creates a dense "digital rain" look instead of readable characters.

### Waterfall Text (`-W "text"`)
Display text as **negative space** through the rain — like Brno fountain displays.
Text appears as dark areas carved into the falling rain.

### Combined Mode
```bash
cmatrix -G -W "HELLO"      # Glitch rain with text
cmatrix -G -W "$(date +%H:%M)" -s  # Live clock through rain
```

## Installation

### Build from source
```bash
# Dependencies (Debian/Ubuntu)
sudo apt install libncurses5-dev libncursesw5-dev

# Build
autoreconf -i
./configure
make
sudo make install
```

### Quick wrapper (uses existing cmatrix)
```bash
# Simple glitch effect via locale hack
sudo cp cmatrix-glitch.sh /usr/local/bin/cmatrix-glitch
sudo chmod +x /usr/local/bin/cmatrix-glitch
```

## Usage

```bash
cmatrix -G                    # Glitch mode (block rain)
cmatrix -G -W "TEXT"          # Glitch + waterfall text
cmatrix -G -s                 # Screensaver mode (exit on keypress)
cmatrix -G -b                 # Bold blocks
cmatrix -G -C cyan            # Custom color
cmatrix -G -r                 # Rainbow mode
```

### All Options
```
-G          Glitch mode (gradient blocks instead of symbols)
-W [text]   Waterfall text display (text as negative space)
-s          Screensaver mode
-b/-B       Bold characters
-C [color]  Color: green, red, blue, cyan, magenta, yellow, white
-r          Rainbow mode
-u [0-10]   Update speed (lower = faster)
-a          Async scroll
-c          Classic Japanese katakana
-m          Lambda mode
```

### Keyboard Controls
| Key | Action |
|-----|--------|
| `q` | Quit |
| `g` | Toggle glitch mode |
| `0-9` | Speed |
| `r` | Rainbow |
| `!@#$%^&` | Colors |
| `p` | Pause |

## How It Works

### Original Glitch Effect
The original `cmatrix-glitch.sh` creates glitch by running:
```bash
LC_ALL=C cmatrix -c
```
This breaks UTF-8 katakana rendering, producing gray/abstract patterns.

### New Glitch Mode (`-G`)
Built-in glitch mode uses Unicode block characters with gradient:
- Head (brightest): `█`
- Upper trail: `▓`
- Middle trail: `▒`
- Lower trail: `░`

This provides predictable, colorful rain effect that works with waterfall text.

## Use Cases

- **Screensaver**: `cmatrix -G -s`
- **tmux lock screen**: `set -g lock-command 'cmatrix -G -s -b'`
- **Status display**: `cmatrix -G -W "SERVER OK"`
- **Clock**: `while true; do cmatrix -G -W "$(date +%H:%M)" -s; done`

## License

GPL-3.0 — same as original CMatrix.

## Credits

- Original CMatrix: Chris Allegretta, Abishek V Ashok
- Glitch & Waterfall extensions: nglain
