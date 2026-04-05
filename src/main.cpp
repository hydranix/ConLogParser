#include <conlog/App.hpp>

#include <exception>
#include <print>

int main()
{
    try {
        conlog::App app;
        return app.run();
    } catch (const std::exception& ex) {
        std::println(stderr, "Fatal: {}", ex.what());
        return 1;
    }
}
