#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include "router.hpp"

class WebSocketServer {
public:
    WebSocketServer(boost::asio::io_context& ioc, unsigned short port, std::shared_ptr<Router> router);
    boost::asio::awaitable<void> run();
private:
    boost::asio::io_context& ioc_;
    unsigned short port_;
    std::shared_ptr<Router> router_;
}; 