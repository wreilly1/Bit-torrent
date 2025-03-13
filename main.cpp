#include <iostream>
#include <string>
#include <unistd.h>
#include "torrent_info.h"
#include "peer_network.h"
#include "bt_messages.h"


int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <torrent_file> <ip:port>\n";
        return 1;
    }

    try {
        // 1) parse .torrent
        TorrentInfo tinfo = loadTorrentInfo(argv[1]);
        std::cout << "Announce:      " << tinfo.announceUrl << "\n"
                  << "Piece length:  " << tinfo.fileData.pieceLength << "\n"
                  << "File size:     " << tinfo.fileData.fileSize << "\n"
                  << "File name:     " << tinfo.fileData.fileName << "\n"
                  << "Piece count:   " << tinfo.fileData.pieceCount << std::endl;

        // 2) parse ip:port
        std::string ipPort(argv[2]);
        auto delim = ipPort.find(':');
        if (delim == std::string::npos) {
            std::cerr << "Invalid ip:port => " << ipPort << std::endl;
            return 1;
        }
        std::string ip = ipPort.substr(0, delim);
        int port = std::stoi(ipPort.substr(delim+1));

        // 3) connect
        int sockfd = connectToPeer(ip, port);

        // 4) handshake
        auto resp = sendHandshakeAndGetResponse(sockfd);
        std::cout << "Handshake response size: " << resp.size() << " bytes\n";

        // (Optionally send bitTorrent messages like choke, unchoke, etc.)
        // auto chokeMsg = buildChokeMessage();
        // send(sockfd, chokeMsg.data(), chokeMsg.size(), 0);

        // 5) run the event loop
        runSelectEventLoop(sockfd);

        ::close(sockfd);
        std::cout << "Client done.\n";
    }
    catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

