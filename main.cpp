#include "router.hpp"

void handler(std::string, int) {

}

int main() {
  router r;
  r.GET<"/api/v1/{string:id}/{int:id2}">(handler);
}
