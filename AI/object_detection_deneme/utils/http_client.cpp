
#include "http_client.h"
#include <iostream>

HttpClient::HttpClient() {
    // Constructor
}

HttpClient::~HttpClient() {
    cleanup();
}

bool HttpClient::initialize() {
    #ifdef _WIN32
    if (!wsaInitialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return false;
        }
        wsaInitialized = true;
    }
    #endif
    return true;
}

void HttpClient::cleanup() {
    #ifdef _WIN32
    if (wsaInitialized) {
        WSACleanup();
        wsaInitialized = false;
    }
    #endif
}

SocketType HttpClient::connectToServer(const std::string& host, int port) {
    // Resolve the server address
    struct addrinfo hints = {}, *addrs;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string portStr = std::to_string(port);
    int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &addrs);
    if (status != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status) << std::endl;
        return INVALID_SOCKET;
    }

    // Create a socket and connect
    SocketType sock = INVALID_SOCKET;
    for (struct addrinfo* addr = addrs; addr != nullptr; addr = addr->ai_next) {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }

        if (connect(sock, addr->ai_addr, (int)addr->ai_addrlen) != SOCKET_ERROR_CODE) {
            break; // Successfully connected
        }

        CLOSE_SOCKET(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(addrs);
    return sock;
}

std::string HttpClient::scansToJsonArray(const std::vector<ScanRequestDTO>& scans) {
    std::string result = "[";
    for (size_t i = 0; i < scans.size(); i++) {
        if (i > 0) {
            result += ",";
        }
        result += scans[i].toJson();
    }
    result += "]";
    return result;
}

bool HttpClient::sendScans(const std::string& host, int port, const std::string& path, 
                          const std::vector<ScanRequestDTO>& scans) {
    // Connect to the server
    SocketType sock = connectToServer(host, port);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }

    // Prepare JSON payload
    std::string jsonPayload = scansToJsonArray(scans);
    
    // Prepare HTTP headers
    std::ostringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << jsonPayload.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << jsonPayload;

    // Send the request
    std::string requestStr = request.str();
    int bytesSent = send(sock, requestStr.c_str(), requestStr.length(), 0);
    if (bytesSent == SOCKET_ERROR_CODE) {
        std::cerr << "send failed" << std::endl;
        CLOSE_SOCKET(sock);
        return false;
    }

    // Receive and check response
    char buffer[4096];
    std::string response;
    bool success = false;

    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }
        buffer[bytesReceived] = '\0';
        response.append(buffer);
        
        // Simple check for success status code (2xx)
        if (response.find("HTTP/1.1 2") != std::string::npos) {
            success = true;
        }
    }

    // Close the socket
    CLOSE_SOCKET(sock);

    return success;
}
