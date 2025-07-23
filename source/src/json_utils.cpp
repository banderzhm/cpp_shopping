#include "json_utils.hpp"

namespace JsonUtils {
    boost::json::value parse(const std::string& str) {
        return boost::json::parse(str);
    }
    std::string stringify(const boost::json::value& val) {
        return boost::json::serialize(val);
    }
} 