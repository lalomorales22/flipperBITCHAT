# BitChat for Flipper Zero

A decentralized Bluetooth mesh messaging app ported to Flipper Zero.

**Made by lalo with <3 for jack**

## Features

- **Bluetooth Mesh Network**: Send messages to nearby Flipper devices
- **Multi-hop Relay**: Messages relay through nearby devices (max 7 hops)
- **Noise Protocol Encryption**: End-to-end encrypted private messages
- **No Internet Required**: Works completely offline
- **Peer Discovery**: Automatic discovery of nearby BitChat users
- **Simple UI**: Text-based interface optimized for Flipper's display

## Architecture

This is a C port of the BitChat protocol, implementing:

- Binary protocol encoder/decoder
- Noise XX handshake for encryption
- BLE transport layer using Flipper's BLE API
- Simple message storage and UI

## Building

Using ufbt (recommended):

```bash
cd flipperBITCHAT
ufbt
```

## Installing

```bash
ufbt launch
```

Or copy the built `.fap` file to your Flipper's `apps/Bluetooth` directory.

## Usage

1. Launch BitChat from Applications > Bluetooth
2. Set your nickname (first launch)
3. Messages from nearby peers will appear automatically
4. Use Flipper keyboard to send messages

## Protocol Compatibility

This implementation is compatible with the main BitChat app (iOS/macOS) and uses the same:
- Binary wire format (v1)
- Noise XX handshake pattern
- BLE service UUID: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C`

## Limitations

Due to Flipper Zero hardware constraints:
- Limited message history (~50 messages)
- No Nostr support (internet-based features)
- No location channels
- Text-only (no media)
- Simplified UI

## License

Public domain (Unlicense) - same as BitChat
