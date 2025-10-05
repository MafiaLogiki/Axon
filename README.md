# Axon

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/status)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-blue.svg)](https://cmake.org/)

A lightweight, high-performance C++20 web framework for building asynchronous HTTP/WebSocket services, with a focus on developer experience and compile-time safety.

## About

Axon is a minimal, modern C++ web library built on top of Boost.Asio and Boost.Beast. It is inspired by the design of elegant and simple routing libraries like Go's `chi`, with the goal of bringing a similar ergonomic feel to the C++ ecosystem.

It enables developers to build fast, scalable web services without the boilerplate and complexity often associated with C++ networking.

## Key Features

- **Modern C++20 Design:** Leverages C++20 features like `constexpr` and non-type template parameters for a clean, macro-free API.
- **Type-Safe Extractors:** A compile-time dependency injection system allows handlers to declaratively request data from the request (path/query parameters, headers, JSON body, application state) directly in their function signature.
- **Expressive Middleware:** A flexible, functional middleware pattern using `std::function` and lambda captures for request processing, logging, authentication, and more.
- **High Performance:** Built directly on the proven performance of Boost.Asio for asynchronous, non-blocking I/O.
- **Header-Only:** Easy to integrate into any CMake project.
- **HTTP/1.1 & WebSocket Support:** Ready for modern web applications.

## Quick Start

Here is a minimal example of a server with a dynamic path parameter.

```cpp
#include <axon/mux.hpp>
#include <axon/router/extract/extract.hpp>

#include <iostream>

int main() {
    boost::asio::io_context io;

    auto mux = axon::mux(io, boost::asio::ip::make_address("127.0.0.1"), 8080);

    mux.GET<"/users/{int:id}">([](axon::response_type& res, axon::router::extract::path<int> id) {
        std::ostream os(res.body());
        os << "User ID: " << std::get<0>(id);
        res.prepare_payload();
    });

    mux.GET<"/">([](axon::response_type& res) {
        boost::beast::ostream os(&res.body());
        os << "Hello, Axon!";
        res.prepare_payload();
    });

    std::cout << "Server starting on http://127.0.0.1:8080" << std::endl;

    mux.serveHTTP();

    io.run();

    return 0;
}
```

## Installation

This is a header-only library, so no pre-compilation is required.

**Requirements:**
*   C++20 compatible compiler (GCC 10+, Clang 12+)
*   Boost (version 1.74+), with components:
    *   `Boost.Asio`
    *   `Boost.Beast`
*   CMake (version 3.15+)

**Build Instructions:**

To build the examples and tests, clone the repository and use CMake:
```bash
git clone https://github.com/MafiaLogiki/axon.git
cd axon
cmake -S . -B build
cmake --build build
```

## Running Tests

To run the tests after building the project, execute the following command from the build directory:
```bash
cd build
ctest
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
