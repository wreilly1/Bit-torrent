#ifndef TORRENT_INFO_H
#define TORRENT_INFO_H

#include <string>
#include <vector>
#include <cstdint>
#include "bencode_parser.h"

/**
 * @brief File-level data from the "info" dict
 */
struct TorrentFileInfo {
    std::vector<uint8_t> infoHash; // 20-byte SHA-1
    int pieceLength      = 0;
    std::string pieceHashes;
    std::string fileName;
    int fileSize         = 0;
    int pieceCount       = 0;
};

/**
 * @brief Top-level torrent info
 */
struct TorrentInfo {
    std::string announceUrl;
    TorrentFileInfo fileData;
    int creationTime    = 0;
    std::string comments;
    std::string createdBy;
    std::string encoding;
};

/**
 * @brief Loads the .torrent file from disk, parses it via BencodeParser,
 *        extracts fields, computes infohash, etc.
 * @throws BtException on parse or I/O errors
 */
TorrentInfo loadTorrentInfo(const std::string &filePath);

#endif // TORRENT_INFO_H

