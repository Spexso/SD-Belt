#include "HttpServerHandler.hpp"

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

std::string HttpServerHandler::handleCommand(const std::string &cmd)
{
    if (!arduino->isConnected())
        return "Arduino not connected.";

    try
    {
        if (cmd == "start")
            return arduino->start(5); // Example ramp rate

        if (cmd == "stop")
            return arduino->stopImmediate();

        if (cmd == "reverse")
            return arduino->setDirection(-1);

        if (cmd == "status")
            return arduino->getStatus();

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

        if (cmd == "dir=1")
            return arduino->setDirection(1);

        if (cmd == "dir=-1")
            return arduino->setDirection(-1);

        return "ERR: Unknown command";
    }
    catch (const std::exception &e)
    {
        return std::string("ERR: Invalid argument - ") + e.what();
    }
}
