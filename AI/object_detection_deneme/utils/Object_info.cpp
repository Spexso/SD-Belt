#include "Object_info.hpp"

// default ctor
object_info::object_info()
    : class_name_{},
      confidence_{0.0},
      x_{0.0},
      y_{0.0},
      is_healthy_{false} {}

// param-ctor
object_info::object_info(const std::string& class_name,
                         double             confidence,
                         double             x,
                         double             y,
                         bool               is_healthy)
    : class_name_{class_name},
      confidence_{confidence},
      x_{x},
      y_{y},
      is_healthy_{is_healthy} {}

// getters
const std::string& object_info::get_class()      const { return class_name_; }
double             object_info::get_confidence() const { return confidence_; }
double             object_info::get_x()          const { return x_; }
double             object_info::get_y()          const { return y_; }
bool               object_info::get_is_healthy() const { return is_healthy_; }

// setters
void object_info::set_class(const std::string& class_name) { class_name_ = class_name; }
void object_info::set_confidence(double confidence)        { confidence_ = confidence; }
void object_info::set_x(double x)                          { x_ = x; }
void object_info::set_y(double y)                          { y_ = y; }
void object_info::set_is_healthy(bool is_healthy)          { is_healthy_ = is_healthy; }



