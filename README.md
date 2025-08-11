# Banime40 ZMK Configuration

ZMK configuration for Banime40 keyboard with nice!nano v2.

## Features
- 40% layout with 4 layers
- Wireless connectivity via Bluetooth LE
- Rotary encoder support (planned)
- Low power consumption

## Flashing
1. Download firmware from GitHub Actions
2. Put nice!nano v2 in bootloader mode
3. Copy `banime40_nice_nano_v2.uf2` to the drive

## Layout
- **Layer 0**: QWERTY base layer
- **Layer 1**: Numbers and symbols  
- **Layer 2**: Base layer repeat
- **Layer 3**: Function keys and navigation

## Building Locally
```bash
west init -l config
west update
west build -s zmk/app -b nice_nano_v2 -- -DSHIELD=banime40
