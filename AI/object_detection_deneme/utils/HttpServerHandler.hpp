#ifndef HTTP_SERVER_HANDLER_HPP
#define HTTP_SERVER_HANDLER_HPP

#include <httplib.h>
#include <iostream>
#include <string>
#include "ArduinoSerial.h" // Make sure this path is correct for your project

class HttpServerHandler
{
public:
    explicit HttpServerHandler(ArduinoSerial* arduinoPtr);

    void Init();
    bool Bind();
    void Start();
    void Stop();

private:
    ArduinoSerial* arduino;
    httplib::Server server;

    std::string handleCommand(const std::string& cmd);
};

#endif // HTTP_SERVER_HANDLER_HPP
