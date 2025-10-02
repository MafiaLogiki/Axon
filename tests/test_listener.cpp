#include <boost/beast/core/ostream.hpp>
#include <sn_router/listener/core.hpp>
#include <sn_router/types.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/url/host_type.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <ostream>
#include <thread>

using namespace boost::beast;
using namespace boost::asio::ip;

TEST(ListenerTest, accept_connection_test) {
  boost::asio::io_context io;
/*
  sn::application_handler fake_app_handler = [](const sn::request_type& req, sn::response_callback_type callback) {
    sn::response_type res{http::status::ok, req.version()};

    boost::beast::ostream(res.body()) << "Hello from test";
    res.prepare_payload();

    callback(res);
  };

  std::shared_ptr<listener::core> listener = std::make_shared<listener::core>(io, boost::asio::ip::make_address("127.0.0.1"), 80);
  listener->set_application_handler(fake_app_handler);

  std::thread server_thread([](auto listener) {
    listener->serveHTTP();
  }, listener);

  try {
    boost::asio::io_context client_ioc;
    tcp::socket client_socket(client_ioc);

    client_socket.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), 80});
    http::write(client_socket, http::request<http::empty_body>{http::verb::get, "/", 11});

    boost::beast::flat_buffer buffer{8192};
    http::response<http::string_body> res;

    http::read(client_socket, buffer, res);

    EXPECT_EQ(res.body(), "Hello from test");
    EXPECT_EQ(res.result(), http::status::ok);

    client_socket.shutdown(tcp::socket::shutdown_both);

    client_ioc.stop();

  } catch(std::exception& e) {
    FAIL() << "Client part failed: " << e.what() << std::endl;
  }

  io.stop();

  if (server_thread.joinable()) {
    server_thread.join();
  }
  */
}
