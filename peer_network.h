#ifndef PEER_NETWORK_H
#define PEER_NETWORK_H

#include <string>

/**
 * @brief Connect to an IP/port via TCP
 * @throws std::runtime_error on error
 * @return socket fd
 */
int connectToPeer(const std::string &ip, int port);

/**
 * @brief Minimal concurrency approach using select() with a single socket.
 */
void runSelectEventLoop(int sockfd);

#endif // PEER_NETWORK_H

