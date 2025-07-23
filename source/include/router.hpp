#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <boost/asio/awaitable.hpp>
#include <boost/json.hpp>

using RequestHandler = std::function<boost::asio::awaitable<boost::json::value>(const boost::json::value&)>;

class Router {
public:
    void add_route(const std::string& method, const std::string& path, RequestHandler handler);
    boost::asio::awaitable<boost::json::value> route(const std::string& method, const std::string& path, const boost::json::value& body);
private:
    std::unordered_map<std::string, std::unordered_map<std::string, RequestHandler>> routes_;
}; 