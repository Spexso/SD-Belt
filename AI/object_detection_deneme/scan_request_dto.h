#ifndef SCAN_REQUEST_DTO_H
#define SCAN_REQUEST_DTO_H

#include <string>

/**
 * C++ equivalent of the Spring Boot ScanRequestDTO record
 */
class ScanRequestDTO {
public:
    // Constructor
    ScanRequestDTO(const std::string& productResult, double confidence, double x, double y)
        : m_productResult(productResult), m_confidence(confidence), m_x(x), m_y(y) {}

    // Getters
    const std::string& productResult() const { return m_productResult; }
    double confidence() const { return m_confidence; }
    double x() const { return m_x; }
    double y() const { return m_y; }

    // Convert to JSON string representation
    std::string toJson() const {
        std::string json = "{";
        json += "\"productResult\":\"" + m_productResult + "\"";
        json += ",\"confidence\":" + std::to_string(m_confidence);
        json += ",\"x\":" + std::to_string(m_x);
        json += ",\"y\":" + std::to_string(m_y);
        json += "}";
        return json;
    }

private:
    std::string m_productResult;
    double m_confidence;
    double m_x;
    double m_y;
};

#endif // SCAN_REQUEST_DTO_H
