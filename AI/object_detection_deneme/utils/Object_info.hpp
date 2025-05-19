#ifndef OBJECT_INFO_HPP
#define OBJECT_INFO_HPP

#include <string>

class object_info {
private:
    std::string class_name_;
    double      confidence_;
    double      x_;
    double      y_;
    bool        is_healthy_;   // new field

public:
    // ctors
    object_info();
    object_info(const std::string& class_name,
                double             confidence,
                double             x,
                double             y,
                bool               is_healthy);

    // getters
    const std::string& get_class()      const;
    double             get_confidence() const;
    double             get_x()          const;
    double             get_y()          const;
    bool               get_is_healthy() const;

    // setters
    void set_class(const std::string& class_name);
    void set_confidence(double confidence);
    void set_x(double x);
    void set_y(double y);
    void set_is_healthy(bool is_healthy);
    
   

};

#endif // OBJECT_INFO_HPP
