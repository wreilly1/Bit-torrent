#include "torrent_info.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <openssl/sha.h>
#include <stdexcept>
#include <sstream>
#include <vector>

/**
 * Bencode re-encoding for the "info" dict
 */
static std::string reencodeDict(const BNode *dictNode);
static void writeBNode(const BNode *node, std::ostringstream &oss);

static BNode* dictLookup(BNode* dictNode, const std::string &key)
{
    if (!dictNode || dictNode->type != BNodeType::DICT) {
        return nullptr;
    }
    auto it = dictNode->dictValue.find(key);
    if (it == dictNode->dictValue.end()) {
        return nullptr;
    }
    return it->second.get();
}

static std::vector<uint8_t> computeSha1(const std::string &data)
{
    std::vector<uint8_t> digest(SHA_DIGEST_LENGTH, 0);
    ::SHA1(reinterpret_cast<const unsigned char*>(data.data()),
           data.size(), digest.data());
    return digest;
}

TorrentInfo loadTorrentInfo(const std::string &filePath)
{
    // read entire file
    struct stat st;
    if (::stat(filePath.c_str(), &st) != 0) {
        throw BtException("loadTorrentInfo: stat failed for " + filePath);
    }
    std::vector<char> buffer(st.st_size);
    int fd = ::open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        throw BtException("loadTorrentInfo: cannot open " + filePath);
    }
    if (::read(fd, buffer.data(), buffer.size()) != (ssize_t)buffer.size()) {
        ::close(fd);
        throw BtException("loadTorrentInfo: read error");
    }
    ::close(fd);

    // parse
    BencodeParser parser;
    auto root = parser.parse(buffer.data(), buffer.size());
    if (!root || root->type != BNodeType::DICT) {
        throw BtException("loadTorrentInfo: top-level not dict");
    }

    TorrentInfo tinfo;

    // gather top-level fields
    if (BNode* ann = dictLookup(root.get(), "announce")) {
        if (ann->type == BNodeType::STRING) {
            tinfo.announceUrl = ann->stringValue;
        }
    }
    if (BNode* ctime = dictLookup(root.get(), "creation date")) {
        if (ctime->type == BNodeType::INT) {
            tinfo.creationTime = (int)ctime->integerValue;
        }
    }
    if (BNode* comm = dictLookup(root.get(), "comment")) {
        if (comm->type == BNodeType::STRING) {
            tinfo.comments = comm->stringValue;
        }
    }
    if (BNode* cby = dictLookup(root.get(), "created by")) {
        if (cby->type == BNodeType::STRING) {
            tinfo.createdBy = cby->stringValue;
        }
    }
    if (BNode* enc = dictLookup(root.get(), "encoding")) {
        if (enc->type == BNodeType::STRING) {
            tinfo.encoding = enc->stringValue;
        }
    }

    // info dict
    BNode* infoDict = dictLookup(root.get(), "info");
    if (!infoDict || infoDict->type != BNodeType::DICT) {
        throw BtException("loadTorrentInfo: missing info dict");
    }

    // re-encode info dict for SHA1
    std::string infoData = reencodeDict(infoDict);
    tinfo.fileData.infoHash = computeSha1(infoData);

    // subfields
    if (BNode* pl = dictLookup(infoDict, "piece length")) {
        if (pl->type == BNodeType::INT) {
            tinfo.fileData.pieceLength = (int)pl->integerValue;
        }
    }
    if (BNode* pieces = dictLookup(infoDict, "pieces")) {
        if (pieces->type == BNodeType::STRING) {
            tinfo.fileData.pieceHashes = pieces->stringValue;
        }
    }
    if (BNode* nm = dictLookup(infoDict, "name")) {
        if (nm->type == BNodeType::STRING) {
            tinfo.fileData.fileName = nm->stringValue;
        }
    }
    if (BNode* fl = dictLookup(infoDict, "length")) {
        if (fl->type == BNodeType::INT) {
            tinfo.fileData.fileSize = (int)fl->integerValue;
        }
    }

    // pieceCount
    if (tinfo.fileData.pieceLength > 0) {
        double fs = (double)tinfo.fileData.fileSize;
        double plf= (double)tinfo.fileData.pieceLength;
        tinfo.fileData.pieceCount = (int)std::ceil(fs / plf);
    }

    return tinfo;
}

static std::string reencodeDict(const BNode *dictNode)
{
    if (!dictNode || dictNode->type != BNodeType::DICT) {
        throw BtException("reencodeDict: node not a dictionary");
    }
    std::ostringstream oss;
    oss << 'd';
    for (auto &pair : dictNode->dictValue) {
        const std::string &k = pair.first;
        oss << k.size() << ':' << k;
        writeBNode(pair.second.get(), oss);
    }
    oss << 'e';
    return oss.str();
}

static void writeBNode(const BNode *node, std::ostringstream &oss)
{
    switch (node->type) {
    case BNodeType::DICT:
        oss << 'd';
        for (auto &p : node->dictValue) {
            oss << p.first.size() << ':' << p.first;
            writeBNode(p.second.get(), oss);
        }
        oss << 'e';
        break;
    case BNodeType::LIST:
        oss << 'l';
        for (auto &itm : node->listValue) {
            writeBNode(itm.get(), oss);
        }
        oss << 'e';
        break;
    case BNodeType::INT:
        oss << 'i' << node->integerValue << 'e';
        break;
    case BNodeType::STRING: {
        const std::string &s = node->stringValue;
        oss << s.size() << ':' << s;
        break;
    }
    }
}

