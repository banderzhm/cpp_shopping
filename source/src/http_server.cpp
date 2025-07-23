#include "http_server.hpp"
#include <boost/beast/http.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

HttpServer::HttpServer(boost::asio::io_context& ioc, unsigned short port, std::shared_ptr<Router> router)
    : ioc_(ioc), port_(port), router_(router) {}

boost::asio::awaitable<void> HttpServer::run() {
    tcp::acceptor acceptor(ioc_, {tcp::v4(), port_});
    for (;;) {
        auto socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
        boost::asio::co_spawn(ioc_, [this, s = std::move(socket)]() mutable -> boost::asio::awaitable<void> {
            try {
                boost::beast::flat_buffer buffer;
                http::request<http::string_body> req;
                co_await http::async_read(s, buffer, req, boost::asio::use_awaitable);
                boost::json::value body;
                if (!req.body().empty()) {
                    body = boost::json::parse(req.body());
                }
                auto res_json = co_await router_->route(std::string(req.method_string()), std::string(req.target()), body);
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::content_type, "application/json");
                res.body() = boost::json::serialize(res_json);
                res.prepare_payload();
                co_await http::async_write(s, res, boost::asio::use_awaitable);
            } catch (std::exception& e) {
                std::cerr << "HTTP session error: " << e.what() << std::endl;
            }
        }, boost::asio::detached);
    }
} 