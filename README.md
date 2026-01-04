# ESPHomario - Super Mario Bros on ESPHome

A fully playable Super Mario Bros game running on ESP32 with ESPHome, featuring authentic NES-style audio using a custom NSF audio player.

## Features

- **Full platformer gameplay** with procedurally generated levels
- **Authentic NES audio** - Music and sound effects using NSF format
- **Power-ups** - Mushrooms and fire flowers
- **Enemies** - Goombas, Koopas, and Piranha Plants
- **Multiple worlds and levels** with increasing difficulty
- **Smooth scrolling** and physics
- **I2S audio output** via MAX98357A amplifier

## Hardware Requirements

- ESP32 development board
- SSD1306 128x64 OLED display (I2C)
- MAX98357A I2S audio amplifier
- 4 buttons for controls (Up/Jump, Down/Duck, Left, Right, Run/Fire)
- Optional: Additional button for Run/Fire

## Wiring

### Display (I2C)
- SDA: GPIO 21
- SCL: GPIO 22

### Audio (I2S)
- BCLK: GPIO 26
- LRC: GPIO 25
- DIN: GPIO 27

### Controls
- Jump/Up: GPIO 32
- Duck/Down: GPIO 33
- Left: GPIO 14
- Right: GPIO 12
- Run/Fire: GPIO 13

## Installation

1. Install ESPHome
2. Copy the configuration files to your ESPHome config directory
3. Update the WiFi credentials in `esphomario-game.yaml`
4. Flash to your ESP32:
   ```bash
   esphome run esphomario-game.yaml
   ```

## Game Controls

- **Jump Button**: Jump (hold for higher jump)
- **Left/Right**: Move Mario
- **Run Button**: Run faster / Shoot fireballs (when powered up)
- **Duck Button**: Duck (when big Mario)

## Audio System

The game uses a custom NSF (NES Sound Format) audio player that emulates the NES APU (Audio Processing Unit):
- 2 Pulse wave channels
- 1 Triangle wave channel
- 1 Noise channel

Music tracks:
- Track 1: Main game theme (loops)
- Track 2: Flagpole victory music (plays once)
- Track 3: Death music (plays once)

Sound effects:
- Jump, coin, power-up, brick break, fireball, jump on enemy, flagpole

## Files

- `esphomario-game.yaml` - Main ESPHome configuration
- `esphomario.h` - Game logic and rendering
- `components/nsf_audio/` - Custom NSF audio component
  - `nsf_audio.h/cpp` - Audio player implementation
  - `apu.h` - NES APU emulation
  - `sfx_data.h` - Sound effects and music data

## Credits

- Game inspired by Super Mario Bros (Nintendo)
- NSF audio format and music data from various NES homebrew projects
- Built with ESPHome framework

## License

This is a fan project for educational purposes. Nintendo owns all rights to Super Mario Bros.
