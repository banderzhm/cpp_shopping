#pragma once
#include <boost/json.hpp>
#include <string>

namespace JsonUtils {
    boost::json::value parse(const std::string& str);
    std::string stringify(const boost::json::value& val);
} 