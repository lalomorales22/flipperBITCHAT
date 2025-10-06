/**
 * BitChat BLE Transport Layer
 * Handles Bluetooth LE communication using Flipper's BLE stack
 */

#pragma once

#include <furi.h>
#include <furi_hal_bt.h>
#include "../bitchat_app.h"

// BLE Service UUIDs (matching BitChat iOS/macOS)
// Mainnet UUID: F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C
#define BITCHAT_SERVICE_UUID \
    { 0x5C, 0x4B, 0x3A, 0x2C, 0x1D, 0x8E, 0x3F, 0x9B, \
      0x5A, 0x4C, 0x9E, 0x4A, 0x2D, 0x5E, 0x7B, 0xF4 }

// Characteristic UUID: A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D
#define BITCHAT_CHAR_UUID \
    { 0x5D, 0x4C, 0x3B, 0x2A, 0x1F, 0x0E, 0x9D, 0x8C, \
      0x5B, 0x4A, 0xF6, 0xE5, 0xD4, 0xC3, 0xB2, 0xA1 }

#define BITCHAT_BLE_MTU 512
#define BITCHAT_BLE_MAX_PEERS 8

typedef struct BitchatBle BitchatBle;

/**
 * Peer connection information
 */
typedef struct {
    uint8_t peer_id[8];
    char nickname[32];
    bool connected;
    uint32_t last_seen;
} BitchatBlePeer;

/**
 * Initialize BLE service
 * @param event_queue Message queue for events
 * @return BLE service instance
 */
BitchatBle* bitchat_ble_alloc(FuriMessageQueue* event_queue);

/**
 * Free BLE service
 */
void bitchat_ble_free(BitchatBle* ble);

/**
 * Start BLE advertising and scanning
 * @param ble BLE service instance
 * @param identity Local identity for advertising
 */
void bitchat_ble_start(BitchatBle* ble, BitchatIdentity* identity);

/**
 * Stop BLE advertising and scanning
 */
void bitchat_ble_stop(BitchatBle* ble);

/**
 * Send a packet to all connected peers (broadcast)
 * @param ble BLE service instance
 * @param data Packet data
 * @param size Packet size
 * @return true on success
 */
bool bitchat_ble_broadcast(BitchatBle* ble, const uint8_t* data, size_t size);

/**
 * Send a packet to a specific peer
 * @param ble BLE service instance
 * @param peer_id Peer ID (8 bytes)
 * @param data Packet data
 * @param size Packet size
 * @return true on success
 */
bool bitchat_ble_send_to_peer(BitchatBle* ble, const uint8_t* peer_id, const uint8_t* data, size_t size);

/**
 * Get list of connected peers
 * @param ble BLE service instance
 * @param peers Output array
 * @param max_peers Maximum number of peers
 * @return Number of peers
 */
size_t bitchat_ble_get_peers(BitchatBle* ble, BitchatBlePeer* peers, size_t max_peers);

/**
 * Check if BLE is active
 */
bool bitchat_ble_is_active(BitchatBle* ble);
