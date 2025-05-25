#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define SOCKET_ERROR_CODE SOCKET_ERROR
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int SocketType;
    #define SOCKET_ERROR_CODE -1
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET(s) close(s)
#endif

#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include "scan_request_dto.h"
#include "system_status_dto.h"
#include "system_messages_dto.h"


class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    //SocketType connectToServer(const std::string& host, int port);

	bool sendSystemStatus(const std::string& host, int port, const std::string& path, 
                         const SystemStatusDTO& status);
                         
    bool sendSystemMessage(const std::string& host, int port, const std::string& path, 
                         const SystemLogMessageDTO& message);


    // Initialize socket library (required for Windows)
    bool initialize();
    
    // Clean up resources
    void cleanup();
    
    // Send scan data to server
    bool sendScans(const std::string& host, int port, const std::string& path, 
                  const std::vector<ScanRequestDTO>& scans);

private:
    // Connect to server
    SocketType connectToServer(const std::string& host, int port);
    
    // Convert vector of ScanRequestDTO to JSON array
    std::string scansToJsonArray(const std::vector<ScanRequestDTO>& scans);
    
    #ifdef _WIN32
    bool wsaInitialized = false;
    #endif
};

#endif // HTTP_CLIENT_H
