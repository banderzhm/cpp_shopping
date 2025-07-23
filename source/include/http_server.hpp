#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <functional>
#include <string>
#include "router.hpp"

class HttpServer {
public:
    HttpServer(boost::asio::io_context& ioc, unsigned short port, std::shared_ptr<Router> router);
    boost::asio::awaitable<void> run();
private:
    boost::asio::io_context& ioc_;
    unsigned short port_;
    std::shared_ptr<Router> router_;
}; 