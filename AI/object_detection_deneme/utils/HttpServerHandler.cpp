#include "HttpServerHandler.hpp"


extern double threshold;

HttpServerHandler::HttpServerHandler(ArduinoSerial *arduinoPtr)
    : arduino(arduinoPtr) {}

void HttpServerHandler::Init()
{
    server.set_logger([](const httplib::Request &req, const httplib::Response &res)
    {
        std::cout << "[HTTP] " << req.method << " " << req.path
                  << " => " << res.status << std::endl;

        if (!req.body.empty())
        {
            std::cout << "  Body: " << req.body << std::endl;
        }
    });

    server.Post("/rev", [this](const httplib::Request &, httplib::Response &res)
    {
        std::string response = handleCommand("reverse");
        std::cout << "[/rev] Response: " << response << std::endl;
        res.set_content(response, "text/plain");
    });

    server.Post("/stop", [this](const httplib::Request &, httplib::Response &res)
    {
        std::string response = handleCommand("stop");
        std::cout << "[/stop] Response: " << response << std::endl;
        res.set_content(response, "text/plain");
    });

    server.Post("/speed", [this](const httplib::Request &req, httplib::Response &res)
    {
        std::string body = req.body;
        std::cout << "[/speed] Received body: " << body << std::endl;

        try
        {
            int percent = std::stoi(body);
            std::string response = arduino->setSpeed(percent);
            std::cout << "[/speed] Response: " << response << std::endl;
            res.set_content(response, "text/plain");
        }
        catch (const std::exception &e)
        {
            std::string error = "ERR: Invalid speed value - " + std::string(e.what());
            std::cerr << "[/speed] " << error << std::endl;
            res.set_content(error, "text/plain");
            res.status = 400;
        }
    });
    
    
    server.Post("/threshold", [this](const httplib::Request &req, httplib::Response &res)
    {
        std::string body = req.body;
        std::cout << "[/threshold] Received body: " << body << std::endl;

        try
        {
            double percent = std::stod(body);
            threshold = percent;
            std::string response = "Threshold changed";
            std::cout << "[/threshold] Response: " << response << std::endl;
            res.set_content(response, "text/plain");
            std::cout << "New threshold value:" << threshold << std::endl;
        }
        catch (const std::exception &e)
        {
            std::string error = "ERR: Invalid threshold value - " + std::string(e.what());
            std::cerr << "[/threshold] " << error << std::endl;
            res.set_content(error, "text/plain");
            res.status = 400;
        }
    });
}

void HttpServerHandler::Start()
{
    std::cout << "API SERVER running at http://0.0.0.0:8080\n";
    server.listen_after_bind();
}

void HttpServerHandler::Stop()
{
    server.stop();
}

bool HttpServerHandler::Bind()
{
    return server.bind_to_port("0.0.0.0", 8080);
}

#include <unordered_map>

std::string HttpServerHandler::handleCommand(const std::string &cmd)
{
    if (!arduino->isConnected())
        return "Arduino not connected.";

    // Define command enum
    enum CommandType {
        CMD_START,
        CMD_STOP,
        CMD_REVERSE,
        CMD_STATUS,
        CMD_SPEED,
        CMD_SERVO,
        CMD_DIR_POS,
        CMD_DIR_NEG,
        CMD_UNKNOWN
    };

    // Map exact matches
    static const std::unordered_map<std::string, CommandType> commandMap = {
        {"start", CMD_START},
        {"stop", CMD_STOP},
        {"reverse", CMD_REVERSE},
        {"status", CMD_STATUS},
        {"dir=1", CMD_DIR_POS},
        {"dir=-1", CMD_DIR_NEG}
    };

    try
    {
        // Handle special prefix commands
        if (cmd.rfind("speed=", 0) == 0)
        {
            int pct = std::stoi(cmd.substr(6));
            return arduino->setSpeed(pct);
        }
        if (cmd.rfind("servo=", 0) == 0)
        {
            int angle = std::stoi(cmd.substr(6));
            return arduino->setServoAngle(angle);
        }

        // Handle exact match commands
        auto it = commandMap.find(cmd);
        CommandType commandType = (it != commandMap.end()) ? it->second : CMD_UNKNOWN;

        switch (commandType)
        {
            case CMD_START:
                return arduino->start(5); // Example ramp rate
            case CMD_STOP:
                return arduino->stopImmediate();
            case CMD_REVERSE:
                return arduino->reverseDirection();
            case CMD_STATUS:
                return arduino->getStatus();
            case CMD_DIR_POS:
                return arduino->setDirection(1);
            case CMD_DIR_NEG:
                return arduino->setDirection(-1);
            case CMD_UNKNOWN:
            default:
                return "ERR: Unknown command";
        }
    }
    catch (const std::exception &e)
    {
        return std::string("ERR: Invalid argument - ") + e.what();
    }
}
