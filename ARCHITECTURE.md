# BitChat Flipper Zero Architecture

## Overview

This is a C implementation of the BitChat protocol for the Flipper Zero platform. It maintains protocol compatibility with the main BitChat app (iOS/macOS) while adapting to Flipper's constraints.

## Directory Structure

```
flipperBITCHAT/
├── protocol/          # Binary protocol encoder/decoder
│   ├── bitchat_protocol.h
│   └── bitchat_protocol.c
├── ble/               # Bluetooth LE transport
│   ├── bitchat_ble.h
│   └── bitchat_ble.c
├── crypto/            # Cryptographic primitives (TODO)
│   ├── noise_protocol.h
│   └── noise_protocol.c
├── storage/           # Identity and message storage
│   └── bitchat_identity.c
├── ui/                # User interface (TODO)
│   ├── chat_view.h
│   └── chat_view.c
├── utils/             # Utility functions (TODO)
├── bitchat_app.c      # Main application
├── bitchat_app.h      # Main header
└── application.fam    # Flipper app manifest
```

## Protocol Layers

### 1. Binary Protocol (`protocol/`)

Implements the BitChat wire format:
- **Header**: 13 bytes (version, type, TTL, timestamp, flags, payload length)
- **Sender ID**: 8 bytes
- **Recipient ID**: 8 bytes (optional)
- **Payload**: Variable length (max 65535 bytes)
- **Signature**: 64 bytes (optional)

Key functions:
- `bitchat_packet_encode()` - Encode packet to binary
- `bitchat_packet_decode()` - Decode binary to packet
- `bitchat_message_encode()` - Encode message to payload
- `bitchat_message_decode()` - Decode payload to message

### 2. BLE Transport (`ble/`)

Handles Bluetooth LE communication:
- Uses Flipper's BLE stack (`furi_hal_bt`)
- Advertises BitChat service UUID
- Scans for nearby peers
- Handles fragmentation for large packets
- Manages peer connections

Key functions:
- `bitchat_ble_start()` - Start advertising/scanning
- `bitchat_ble_broadcast()` - Broadcast to all peers
- `bitchat_ble_send_to_peer()` - Send to specific peer
- `bitchat_ble_get_peers()` - Get connected peers

### 3. Cryptography (`crypto/`) - TODO

Will implement:
- **Noise Protocol XX handshake**
  - Uses mbedtls for Curve25519, ChaCha20-Poly1305
  - Session management
  - Key derivation
- **Ed25519 signatures** for announcements
- **SHA-256** for fingerprints

### 4. Identity Management (`storage/`)

Manages cryptographic identity:
- Generates Noise static key pair (Curve25519)
- Generates signing key pair (Ed25519)
- Derives peer ID from public key
- Stores identity in Flipper storage
- Manages nickname

### 5. User Interface (`ui/`) - TODO

Simple text-based UI:
- Chat view with message list
- Peer list
- Settings view
- Text input for messages

## Message Flow

### Sending a Public Message

1. User enters message in UI
2. Create `BitchatMessage` structure
3. Encode message to binary payload
4. Create `BitchatPacket` with type=PUBLIC_MESSAGE
5. Encode packet to binary wire format
6. Send via BLE to all connected peers (broadcast)
7. Each peer relays if TTL > 0

### Receiving a Message

1. BLE layer receives data
2. Stream assembler combines fragments
3. Decode binary to `BitchatPacket`
4. Check TTL, decrement if > 0
5. Decode payload to `BitchatMessage`
6. Display in UI
7. If TTL > 0, relay to other peers

### Private Message (Encrypted)

1. User selects peer, enters message
2. Look up Noise session for peer
3. Encrypt message with Noise
4. Create packet with type=PRIVATE_MESSAGE, recipient_id set
5. Send via BLE to target peer (or broadcast if not directly connected)
6. Recipient decrypts with Noise session

## Protocol Compatibility

Matches BitChat v1 protocol:
- Same binary wire format
- Same BLE service UUID: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C`
- Same characteristic UUID: `A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D`
- Same Noise XX pattern
- Compatible with iOS/macOS BitChat app

## Memory Constraints

Flipper Zero limitations:
- **RAM**: ~256 KB
- **Storage**: Limited (SD card available)
- **BLE MTU**: 512 bytes typical

Optimizations:
- Limited message history (50-100 messages)
- No compression (to save code size)
- Simple peer cache
- Streaming packet assembly

## Security Considerations

- Private keys stored in Flipper storage (encrypted by OS)
- Noise provides forward secrecy
- No persistence of session keys (regenerate on app start)
- Simplified crypto (using mbedtls)

## Building

```bash
cd flipperBITCHAT
ufbt          # Build
ufbt launch   # Flash and launch
```

## TODO

- [ ] Implement Noise Protocol handshake
- [ ] Add ChaCha20-Poly1305 encryption
- [ ] Implement UI views
- [ ] Add message history storage
- [ ] Implement packet fragmentation
- [ ] Add peer discovery
- [ ] Implement message relay
- [ ] Add delivery acknowledgments
- [ ] Test with iOS BitChat app
- [ ] Add icon and assets
