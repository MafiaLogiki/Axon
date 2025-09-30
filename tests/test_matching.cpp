#include <gtest/gtest.h>
#include <router.hpp>
#include <boost/beast.hpp>

TEST(matching_test, simple_match_test) {
  boost::asio::io_context io;

  auto r = Router::create_router(io);

  r->GET<"/test/path">([](){

  });
}
