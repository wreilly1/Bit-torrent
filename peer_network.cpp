
#include "peer_network.h"
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

int connectToPeer(const std::string &ip, int port)
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("connectToPeer: socket() failed");
    }

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);

    if (::inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        ::close(sockfd);
        throw std::runtime_error("connectToPeer: inet_pton failed");
    }

    if (::connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        ::close(sockfd);
        throw std::runtime_error(std::string("connectToPeer: connect() failed => ") + std::strerror(errno));
    }

    std::cerr << "[INFO ] Connected to " << ip << ":" << port << std::endl;
    return sockfd;
}

/**
 * @brief Minimal select-based event loop for a single connection
 */
void runSelectEventLoop(int sockfd)
{
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        std::cerr << "[DEBUG] Waiting on select()...\n";
        int ret = ::select(sockfd+1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            std::cerr << "[ERROR] select() error\n";
            break;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            uint8_t buffer[1024];
            int n = ::recv(sockfd, buffer, sizeof(buffer), 0);
            if (n <= 0) {
                std::cerr << "[INFO ] Peer closed or error => exiting loop\n";
                break;
            }
            std::cerr << "[DEBUG] Received " << n << " bytes\n";
        }
    }

    std::cerr << "[INFO ] Event loop ended\n";
}

