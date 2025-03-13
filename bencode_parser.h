#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

/**
 * @brief Our custom exception type for bencode/torrent errors
 */
class BtException : public std::runtime_error
{
public:
    explicit BtException(const std::string &msg)
        : std::runtime_error(msg) {}
};

/**
 * @brief Enum for node types in our bencode parse tree
 */
enum class BNodeType {
    DICT,
    LIST,
    INT,
    STRING
};

/**
 * @brief BNode: one node in the bencode tree
 */
class BNode
{
public:
    BNodeType type;

    long integerValue = 0;          // for INT
    std::string stringValue;        // for STRING
    std::vector<std::unique_ptr<BNode>> listValue; // for LIST
    std::unordered_map<std::string, std::unique_ptr<BNode>> dictValue; // for DICT

    explicit BNode(BNodeType t) : type(t) {}
};

/**
 * @brief A parser that builds a bencode tree (BNode) from raw data
 */
class BencodeParser
{
public:
    /**
     * @brief Parse the given buffer into a single root BNode
     * @throws BtException on malformed data
     */
    std::unique_ptr<BNode> parse(const char *data, size_t length);

private:
    const char *m_data   = nullptr;
    size_t      m_length = 0;
    size_t      m_pos    = 0;

    // Helper functions
    char peekChar() const;
    char getChar();
    bool atEnd() const;
    void expectChar(char c);

    std::unique_ptr<BNode> parseOne();
    std::unique_ptr<BNode> parseDict();
    std::unique_ptr<BNode> parseList();
    std::unique_ptr<BNode> parseInt();
    std::unique_ptr<BNode> parseString();
};

#endif // BENCODE_PARSER_H

