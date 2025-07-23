#include "websocket_server.hpp"
#include <boost/beast/websocket.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc, unsigned short port, std::shared_ptr<Router> router)
    : ioc_(ioc), port_(port), router_(router) {}

boost::asio::awaitable<void> WebSocketServer::run() {
    tcp::acceptor acceptor(ioc_, {tcp::v4(), port_});
    for (;;) {
        auto socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
        boost::asio::co_spawn(ioc_, [this, s = std::move(socket)]() mutable -> boost::asio::awaitable<void> {
            try {
                websocket::stream<tcp::socket> ws(std::move(s));
                co_await ws.async_accept(boost::asio::use_awaitable);
                for (;;) {
                    boost::beast::flat_buffer buffer;
                    co_await ws.async_read(buffer, boost::asio::use_awaitable);
                    std::string msg = boost::beast::buffers_to_string(buffer.data());
                    auto req_json = boost::json::parse(msg);
                    auto res_json = co_await router_->route("WS", "/ws", req_json);
                    std::string res_str = boost::json::serialize(res_json);
                    co_await ws.async_write(boost::asio::buffer(res_str), boost::asio::use_awaitable);
                }
            } catch (std::exception& e) {
                std::cerr << "WebSocket session error: " << e.what() << std::endl;
            }
        }, boost::asio::detached);
    }
} 