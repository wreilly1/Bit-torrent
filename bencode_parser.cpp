#include "bencode_parser.h"
#include <cctype>
#include <sstream>

std::unique_ptr<BNode> BencodeParser::parse(const char *data, size_t length)
{
    m_data   = data;
    m_length = length;
    m_pos    = 0;
    return parseOne();
}

char BencodeParser::peekChar() const
{
    if (atEnd()) {
        throw BtException("BencodeParser: unexpected end of data");
    }
    return m_data[m_pos];
}

char BencodeParser::getChar()
{
    char c = peekChar();
    m_pos++;
    return c;
}

bool BencodeParser::atEnd() const
{
    return (m_pos >= m_length);
}

void BencodeParser::expectChar(char c)
{
    char actual = getChar();
    if (actual != c) {
        std::ostringstream oss;
        oss << "BencodeParser: expected '" << c
            << "', got '" << actual << "'";
        throw BtException(oss.str());
    }
}

std::unique_ptr<BNode> BencodeParser::parseOne()
{
    char c = peekChar();
    switch (c) {
    case 'd': return parseDict();
    case 'l': return parseList();
    case 'i': return parseInt();
    default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
            return parseString();
        }
        break;
    }
    std::ostringstream oss;
    oss << "BencodeParser: invalid start character '" << c << "'";
    throw BtException(oss.str());
}

std::unique_ptr<BNode> BencodeParser::parseDict()
{
    expectChar('d');
    auto node = std::make_unique<BNode>(BNodeType::DICT);

    while (!atEnd() && peekChar() != 'e') {
        // read dict key
        if (!std::isdigit(static_cast<unsigned char>(peekChar()))) {
            throw BtException("BencodeParser: dict key not digit-based string");
        }
        long keyLen = 0;
        while (std::isdigit(static_cast<unsigned char>(peekChar()))) {
            keyLen = keyLen * 10 + (getChar() - '0');
        }
        expectChar(':');
        if (m_pos + keyLen > m_length) {
            throw BtException("BencodeParser: dict key extends beyond buffer");
        }

        std::string dictKey(m_data + m_pos, keyLen);
        m_pos += keyLen;

        // parse the value
        auto val = parseOne();
        node->dictValue.emplace(dictKey, std::move(val));
    }
    expectChar('e');
    return node;
}

std::unique_ptr<BNode> BencodeParser::parseList()
{
    expectChar('l');
    auto node = std::make_unique<BNode>(BNodeType::LIST);

    while (!atEnd() && peekChar() != 'e') {
        auto item = parseOne();
        node->listValue.push_back(std::move(item));
    }
    expectChar('e');
    return node;
}

std::unique_ptr<BNode> BencodeParser::parseInt()
{
    expectChar('i');
    bool negative = false;
    if (peekChar() == '-') {
        negative = true;
        getChar(); // skip '-'
    }
    long val = 0;
    bool hasDigits = false;
    while (!atEnd() && std::isdigit(static_cast<unsigned char>(peekChar()))) {
        hasDigits = true;
        val = val * 10 + (getChar() - '0');
    }
    if (!hasDigits) {
        throw BtException("BencodeParser: integer has no digits");
    }
    if (negative) {
        val = -val;
    }
    expectChar('e');

    auto node = std::make_unique<BNode>(BNodeType::INT);
    node->integerValue = val;
    return node;
}

std::unique_ptr<BNode> BencodeParser::parseString()
{
    long lengthNum = 0;
    while (std::isdigit(static_cast<unsigned char>(peekChar()))) {
        lengthNum = lengthNum * 10 + (getChar() - '0');
    }
    expectChar(':');
    if (m_pos + lengthNum > m_length) {
        throw BtException("BencodeParser: string extends beyond buffer");
    }

    std::string s(m_data + m_pos, lengthNum);
    m_pos += lengthNum;

    auto node = std::make_unique<BNode>(BNodeType::STRING);
    node->stringValue = s;
    return node;
}

