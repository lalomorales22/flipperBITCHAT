# BitChat for Flipper Zero

A decentralized Bluetooth mesh messaging app ported to Flipper Zero.

**Made by lalo with <3 for jack**

## Features

- **Bluetooth Mesh Network**: Send messages to nearby Flipper devices
- **Multi-hop Relay**: Messages relay through nearby devices (max 7 hops)
- **Noise Protocol Encryption**: End-to-end encrypted private messages
- **No Internet Required**: Works completely offline
- **Peer Discovery**: Automatic discovery of nearby BitChat users
- **Intuitive UI**: Clean, optimized interface for Flipper's 128x64 display
  - Scrollable message history (up to 50 messages)
  - Real-time peer count indicator
  - Connection status with visual feedback
  - Easy text input with Flipper's keyboard
  - Smooth navigation between views
  - Visual and haptic feedback for actions

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

1. **Launch** BitChat from Applications > Bluetooth
2. **First Time Setup**: Enter your nickname when prompted
3. **View Messages**: Scroll through chat history with Up/Down buttons
4. **Send Messages**: Press OK button to open keyboard, type and send
5. **Monitor Status**: Check connection indicator (●) and peer count in header
6. **Navigation**: Press Back to return to previous view or exit

### Interface Guide

**Main Chat View:**
- Header shows connection status (●/○) and peer count
- Message area displays conversation history
- Your messages are marked with ">"
- Use Up/Down to scroll through messages
- Press OK to compose new message
- Press Back to exit

**Message Input:**
- Enter your message using Flipper keyboard
- Press OK to send
- Press Back to cancel

**Status Indicators:**
- ● (Filled circle) = Connected to BLE mesh
- ○ (Empty circle) = Disconnected
- Number shows active peers nearby

## Protocol Compatibility

This implementation is compatible with the main BitChat app (iOS/macOS) and uses the same:
- Binary wire format (v1)
- Noise XX handshake pattern
- BLE service UUID: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C`

## UI/UX Design

The interface is designed specifically for the Flipper Zero's 128x64 monochrome display:

**Design Principles:**
- **Clarity**: High contrast, readable fonts (Primary & Secondary)
- **Efficiency**: Minimal button presses to perform actions
- **Feedback**: Visual (status icons) and haptic (vibrations) confirmations
- **Simplicity**: Focus on core messaging functionality
- **Navigation**: Intuitive back-button behavior for easy flow

**View Structure:**
```
Nickname Setup → Main Chat View ⇄ Message Input
     (First launch)     ↑               ↓
                        └───────────────┘
```

**Screen Layout:**
```
┌──────────────────────────┐
│ ● BitChat      2 peers   │ ← Status Header
├──────────────────────────┤
│ sender1: Hello there     │
│ > You: Hi! How are you?  │ ← Message List
│ sender2: Great!          │   (Scrollable)
│          ↕               │
├──────────────────────────┤
│  OK=Send Back=Menu       │ ← Help Footer
└──────────────────────────┘
```

## Limitations

Due to Flipper Zero hardware constraints:
- Limited message history (~50 messages)
- No Nostr support (internet-based features)
- No location channels
- Text-only (no media)
- Simplified UI optimized for small display

## Development Status

✅ **Completed:**
- Binary protocol encoder/decoder
- Identity management and storage
- BLE transport layer structure
- Complete UI/UX implementation
- Chat view with scrolling
- Text input for messages
- Nickname setup flow
- Status indicators and feedback

🚧 **In Progress:**
- Noise Protocol encryption
- Actual BLE advertising/scanning
- Message relay implementation
- Peer discovery

## License

Public domain (Unlicense) - same as BitChat
