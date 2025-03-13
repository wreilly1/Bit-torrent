#include "bt_messages.h"
#include <stdexcept>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h> // for htonl
#include <unistd.h>

std::vector<uint8_t> sendHandshakeAndGetResponse(int sockfd)
{
    // simplified handshake
    const std::string handshake = "SimpleHandshake";
    if (::send(sockfd, handshake.data(), handshake.size(), 0) < 0) {
        throw std::runtime_error("sendHandshakeAndGetResponse: send failed");
    }

    std::vector<uint8_t> resp(64, 0);
    int n = ::recv(sockfd, resp.data(), resp.size(), 0);
    if (n <= 0) {
        throw std::runtime_error("sendHandshakeAndGetResponse: no response from peer");
    }
    resp.resize(n);
    return resp;
}

std::vector<uint8_t> buildChokeMessage()
{
    // typical choke => 5 bytes: [4-byte length=1][1-byte id=0]
    std::vector<uint8_t> msg(5, 0);
    uint32_t netLen = htonl(1); // big-endian
    std::memcpy(msg.data(), &netLen, 4);
    msg[4] = 0; // choke
    return msg;
}

std::vector<uint8_t> buildUnchokeMessage()
{
    std::vector<uint8_t> msg(5, 0);
    uint32_t netLen = htonl(1);
    std::memcpy(msg.data(), &netLen, 4);
    msg[4] = 1; // unchoke
    return msg;
}

std::vector<uint8_t> buildInterestedMessage()
{
    std::vector<uint8_t> msg(5, 0);
    uint32_t netLen = htonl(1);
    std::memcpy(msg.data(), &netLen, 4);
    msg[4] = 2; // interested
    return msg;
}

