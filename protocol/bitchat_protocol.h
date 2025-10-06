/**
 * BitChat Protocol Implementation
 * Binary protocol encoder/decoder matching the Swift implementation
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Protocol constants
#define BITCHAT_VERSION 1
#define BITCHAT_HEADER_SIZE 13
#define BITCHAT_SENDER_ID_SIZE 8
#define BITCHAT_RECIPIENT_ID_SIZE 8
#define BITCHAT_SIGNATURE_SIZE 64
#define BITCHAT_MAX_PAYLOAD_SIZE 65535
#define BITCHAT_MAX_PACKET_SIZE (BITCHAT_HEADER_SIZE + BITCHAT_SENDER_ID_SIZE + \
                                 BITCHAT_RECIPIENT_ID_SIZE + BITCHAT_MAX_PAYLOAD_SIZE + \
                                 BITCHAT_SIGNATURE_SIZE)

// Packet types
typedef enum {
    BITCHAT_PACKET_TYPE_PUBLIC_MESSAGE = 0x01,
    BITCHAT_PACKET_TYPE_PRIVATE_MESSAGE = 0x02,
    BITCHAT_PACKET_TYPE_ANNOUNCEMENT = 0x03,
    BITCHAT_PACKET_TYPE_SYNC_REQUEST = 0x04,
    BITCHAT_PACKET_TYPE_SYNC_RESPONSE = 0x05,
    BITCHAT_PACKET_TYPE_NOISE_HANDSHAKE = 0x06,
    BITCHAT_PACKET_TYPE_DELIVERY_ACK = 0x07,
} BitchatPacketType;

// Flag bits
#define BITCHAT_FLAG_HAS_RECIPIENT 0x01
#define BITCHAT_FLAG_HAS_SIGNATURE 0x02
#define BITCHAT_FLAG_IS_COMPRESSED 0x04

/**
 * BitChat packet structure
 */
typedef struct {
    uint8_t version;
    uint8_t type;
    uint8_t ttl;
    uint64_t timestamp;
    uint8_t flags;
    uint16_t payload_length;
    uint8_t sender_id[BITCHAT_SENDER_ID_SIZE];
    uint8_t recipient_id[BITCHAT_RECIPIENT_ID_SIZE];
    uint8_t* payload;
    uint8_t signature[BITCHAT_SIGNATURE_SIZE];
    bool has_recipient;
    bool has_signature;
    bool is_compressed;
} BitchatPacket;

/**
 * BitChat message structure
 */
typedef struct {
    char id[37];  // UUID string
    char sender[32];
    char content[256];
    uint64_t timestamp;
    bool is_relay;
    bool is_private;
    char original_sender[32];
    char recipient_nickname[32];
    char sender_peer_id[17];  // 8 bytes hex = 16 chars + null
} BitchatMessage;

/**
 * Encode a packet to binary format
 * @param packet The packet to encode
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
size_t bitchat_packet_encode(const BitchatPacket* packet, uint8_t* buffer, size_t buffer_size);

/**
 * Decode binary data to a packet
 * @param data Input binary data
 * @param data_size Size of input data
 * @param packet Output packet structure
 * @return true on success, false on error
 */
bool bitchat_packet_decode(const uint8_t* data, size_t data_size, BitchatPacket* packet);

/**
 * Encode a message to binary payload
 * @param message The message to encode
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
size_t bitchat_message_encode(const BitchatMessage* message, uint8_t* buffer, size_t buffer_size);

/**
 * Decode binary payload to a message
 * @param data Input binary data
 * @param data_size Size of input data
 * @param message Output message structure
 * @return true on success, false on error
 */
bool bitchat_message_decode(const uint8_t* data, size_t data_size, BitchatMessage* message);

/**
 * Create a new packet
 */
BitchatPacket* bitchat_packet_alloc(void);

/**
 * Free a packet
 */
void bitchat_packet_free(BitchatPacket* packet);

/**
 * Create a new message
 */
BitchatMessage* bitchat_message_alloc(void);

/**
 * Free a message
 */
void bitchat_message_free(BitchatMessage* message);

/**
 * Generate a unique message ID (UUID v4)
 */
void bitchat_generate_message_id(char* id, size_t size);

/**
 * Get current timestamp in milliseconds
 */
uint64_t bitchat_get_timestamp_ms(void);
