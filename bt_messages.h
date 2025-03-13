#ifndef BT_MESSAGES_H
#define BT_MESSAGES_H

#include <vector>
#include <cstdint>
#include <string>

/**
 * @brief Minimal handshake (renamed from e.g. bt_handshake).
 *        Real BitTorrent handshake is 68 bytes, but we simplify here.
 * @return The raw bytes from the peer's response
 * @throws std::runtime_error on send/recv issues
 */
std::vector<uint8_t> sendHandshakeAndGetResponse(int sockfd);

/**
 * @brief Example: build a CHOKE message (5 bytes)
 */
std::vector<uint8_t> buildChokeMessage();

/**
 * @brief Example: build an UNCHOKE message
 */
std::vector<uint8_t> buildUnchokeMessage();

/**
 * @brief Example: build an INTERESTED message
 */
std::vector<uint8_t> buildInterestedMessage();

#endif // BT_MESSAGES_H

