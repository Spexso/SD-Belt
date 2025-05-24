#ifndef SYSTEM_STATUS_DTO_H
#define SYSTEM_STATUS_DTO_H

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * C++ equivalent of the Spring Boot SystemStatus record
 */
class SystemStatusDTO {
public:
    // Constructor
    SystemStatusDTO(const std::chrono::system_clock::time_point& timestamp, 
                   double degree, 
                   double cpuUtilization, 
                   const std::string& memoryUsage)
        : m_timestamp(timestamp), m_degree(degree), m_cpuUtilization(cpuUtilization), m_memoryUsage(memoryUsage) {}

    // Alternative constructor with string timestamp (ISO format)
    SystemStatusDTO(const std::string& timestampStr, 
                   double degree, 
                   double cpuUtilization, 
                   const std::string& memoryUsage)
        : m_degree(degree), m_cpuUtilization(cpuUtilization), m_memoryUsage(memoryUsage) {
        // Parse ISO timestamp string to time_point (simplified)
        m_timestamp = std::chrono::system_clock::now(); // Default to current time
    }

    // Getters
    const std::chrono::system_clock::time_point& timestamp() const { return m_timestamp; }
    double degree() const { return m_degree; }
    double cpuUtilization() const { return m_cpuUtilization; }
    const std::string& memoryUsage() const { return m_memoryUsage; }

    // Get timestamp as ISO string
    std::string timestampAsString() const {
        auto time_t = std::chrono::system_clock::to_time_t(m_timestamp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    // Convert to JSON string representation
    std::string toJson() const {
        std::string json = "{";
        json += "\"timestamp\":\"" + timestampAsString() + "\"";
        json += ",\"cpuDegree\":" + std::to_string(m_degree);
        json += ",\"cpuUsage\":" + std::to_string(m_cpuUtilization);
        json += ",\"memoryUsage\":\"" + m_memoryUsage + "\"";
        json += "}";
        return json;
    }

private:
    std::chrono::system_clock::time_point m_timestamp;
    double m_degree;
    double m_cpuUtilization;
    std::string m_memoryUsage;
};

#endif // SYSTEM_STATUS_DTO_H
