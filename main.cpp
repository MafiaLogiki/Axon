#include "router.hpp"

void handler(std::string, int, int);

int main() {
  router r;
  // auto t = r.get_tuple_of_args<"/api/v1/{string:id}/{int:id2}">();
  r.GET<"/api/v1/{string:id}/{int:id2}">(handler);
}
