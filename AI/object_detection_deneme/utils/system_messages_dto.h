#ifndef SYSTEM_LOG_MESSAGE_DTO_H
#define SYSTEM_LOG_MESSAGE_DTO_H

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * C++ DTO for system log messages, such as "INFO: Camera is opened"
 */
class SystemLogMessageDTO 
{
public:
    // Enum for log level
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR,
        DEBUG
    };
    
    SystemLogMessageDTO()
         {}

	

    // Constructor
    SystemLogMessageDTO(LogLevel level, 
                        const std::string& message,
                        const std::chrono::system_clock::time_point& timestamp = std::chrono::system_clock::now())
        : m_level(level), m_message(message), m_timestamp(timestamp) {}

    // Getters
    LogLevel level() const { return m_level; }
    const std::string& message() const { return m_message; }
    const std::chrono::system_clock::time_point& timestamp() const { return m_timestamp; }

    // Convert timestamp to ISO 8601 format
    std::string timestampAsString() const {
        auto time_t = std::chrono::system_clock::to_time_t(m_timestamp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    // Convert log level to string
    std::string logLevelToString() const {
        switch (m_level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::DEBUG: return "DEBUG";
            default: return "UNKNOWN";
        }
    }

    // Convert to combined message string: e.g., "INFO: Camera is opened"
    std::string formattedMessage() const {
        return logLevelToString() + ": " + m_message;
    }

    // Convert to JSON
    std::string toJson() const {
        std::string json = "{";
        json += "\"timestamp\":\"" + timestampAsString() + "\"";
        json += ",\"level\":\"" + logLevelToString() + "\"";
        json += ",\"message\":\"" + m_message + "\"";
        json += "}";
        return json;
    }

private:
    LogLevel m_level;
    std::string m_message;
    std::chrono::system_clock::time_point m_timestamp;
};

#endif // SYSTEM_LOG_MESSAGE_DTO_H
