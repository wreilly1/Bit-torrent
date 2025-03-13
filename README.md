Minimal BitTorrent-Style Client

This repository hosts a simplified BitTorrent-style client in C++. 
Key Highlights:

    Modern C++
        Uses std::unique_ptr, std::vector<std::uint8_t>, and custom exceptions instead of exit() or raw pointers.
        Splits functionality across multiple .cpp and .h files for clarity and maintainability.

    Tree-Based Bencode Parser
        bencode_parser.*: Reads .torrent files by building an internal tree of nodes (BNode objects) for dictionaries, lists, integers, and strings.
        Throws a custom BtException on parse errors.

    Torrent Metadata Extraction
        torrent_info.*: Takes the parse tree, extracts fields like announce, piece length, file size, etc.
        Re-encodes the “info” dictionary to compute the infohash via SHA-1 (the standard BitTorrent approach).

    Minimal Network Concurrency
        peer_network.*: Connects to a single peer (via IP and port), then uses an event loop with select() to handle incoming data.
        This approach can be extended to track multiple peers in the same loop.

    Protocol Messages
        bt_messages.*: Builds/sends a simplified handshake (in a real client, you’d implement the official 68-byte handshake). Also includes example choke/unchoke/interested message builders.

Repository Structure

    bencode_parser.h / bencode_parser.cpp
    Implements a parser that converts raw bencode data into a BNode tree. Each node can be a DICT, LIST, INT, or STRING. Throws BtException on invalid data.

    torrent_info.h / torrent_info.cpp
    Defines TorrentInfo (top-level metadata) and TorrentFileInfo (infohash, piece length, etc.).
        Loads a .torrent file.
        Extracts basic fields (announce URL, creation date, file size…).
        Re-encodes the info dictionary to compute the infohash.

    peer_network.h / peer_network.cpp
        connectToPeer(...): Connects to <ip:port> via TCP.
        runSelectEventLoop(...): Waits for data using select(). On data availability, it recv()s and logs the result. If the peer closes the connection, the loop exits.

    bt_messages.h / bt_messages.cpp
        sendHandshakeAndGetResponse(...): A short placeholder handshake, returning whatever the peer sends back.
        buildChokeMessage(), buildUnchokeMessage(), buildInterestedMessage(): Demonstrate how protocol messages can be formed (5 bytes each). Real clients would implement more message types (e.g. REQUEST, PIECE, etc.).

    main.cpp
        Ties everything together:
            Loads .torrent metadata with loadTorrentInfo(...).
            Connects to the specified peer.
            Sends a simplified handshake and prints the peer’s response length.
            Enters the runSelectEventLoop(...) until the peer closes or an error occurs.

