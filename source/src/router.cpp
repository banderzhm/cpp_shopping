#include "router.hpp"

void Router::add_route(const std::string& method, const std::string& path, RequestHandler handler) {
    routes_[method][path] = handler;
}

boost::asio::awaitable<boost::json::value> Router::route(const std::string& method, const std::string& path, const boost::json::value& body) {
    if (routes_.count(method) && routes_[method].count(path)) {
        co_return co_await routes_[method][path](body);
    }
    co_return boost::json::object{{"error", "route not found"}};
} 