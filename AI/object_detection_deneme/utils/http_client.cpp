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

        // Set socket to non-blocking mode
        #ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        #else
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        #endif

        // Attempt to connect
        int connectResult = connect(sock, addr->ai_addr, (int)addr->ai_addrlen);
        
        #ifdef _WIN32
        if (connectResult == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                CLOSE_SOCKET(sock);
                sock = INVALID_SOCKET;
                continue;
            }
        }
        #else
        if (connectResult == -1 && errno != EINPROGRESS) {
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        #endif

        // Wait for connection with timeout
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(sock, &writeSet);
        
        struct timeval timeout;
        timeout.tv_sec = 2;  // 2 second timeout
        timeout.tv_usec = 0;
        
        int selectResult = select(sock + 1, nullptr, &writeSet, nullptr, &timeout);
        
        if (selectResult > 0 && FD_ISSET(sock, &writeSet)) {
            // Check if connection was successful
            int error = 0;
            socklen_t errorLen = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLen) == 0 && error == 0) {
                // Set socket back to blocking mode
                #ifdef _WIN32
                mode = 0;
                ioctlsocket(sock, FIONBIO, &mode);
                #else
                flags = fcntl(sock, F_GETFL, 0);
                fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
                #endif
                break; // Successfully connected
            }
        }
        
        // Connection failed or timed out
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

    // Set socket timeout for send and receive operations
    struct timeval timeout;
    timeout.tv_sec = 2;  // 2 second timeout
    timeout.tv_usec = 0;
    
    #ifdef _WIN32
    DWORD timeoutMs = 2000; // 2 seconds in milliseconds
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeoutMs, sizeof(timeoutMs));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeoutMs, sizeof(timeoutMs));
    #else
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    #endif

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


bool HttpClient::sendSystemStatus(const std::string& host, int port, const std::string& path, 
                                 const SystemStatusDTO& status) {
    // Connect to the server
    SocketType sock = connectToServer(host, port);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Prepare JSON payload
    std::string jsonPayload = status.toJson();
    
    // Prepare HTTP request
    std::ostringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << jsonPayload.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << jsonPayload;

    // Send request
    std::string requestStr = request.str();
    int bytesSent = send(sock, requestStr.c_str(), requestStr.length(), 0);
    
    CLOSE_SOCKET(sock);
    return bytesSent != SOCKET_ERROR_CODE;
}


bool HttpClient::sendSystemMessage(const std::string& host, int port, const std::string& path, 
                                 const SystemLogMessageDTO& message) 
{
    // Connect to the server
    SocketType sock = connectToServer(host, port);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Prepare JSON payload
    std::string jsonPayload = message.toJson();
    
    // Prepare HTTP request
    std::ostringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << jsonPayload.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << jsonPayload;

    // Send request
    std::string requestStr = request.str();
    int bytesSent = send(sock, requestStr.c_str(), requestStr.length(), 0);
    
    CLOSE_SOCKET(sock);
    return bytesSent != SOCKET_ERROR_CODE;
}
