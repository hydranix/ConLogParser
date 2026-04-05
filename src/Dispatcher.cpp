#include <conlog/core/Dispatcher.hpp>
#include <conlog/parser/ParserRegistry.hpp>

#include <algorithm>
#include <exception>
#include <execution>
#include <print>

namespace conlog {

Dispatcher::Dispatcher(ParserRegistry& registry)
    : m_registry(registry)
{}

void Dispatcher::dispatch(std::string_view line)
{
    auto handles = m_registry.snapshot();
    if (handles.empty()) return;

    std::for_each(std::execution::par, handles.begin(), handles.end(),
        [line](const ParserHandle& h) {
            try {
                h.invoke(line);
            } catch (const std::exception& ex) {
                // Exceptions in parallel algorithms cause std::terminate.
                // Catch and log instead.
                std::println(stderr, "Parser exception: {}", ex.what());
            } catch (...) {
                std::println(stderr, "Parser threw unknown exception");
            }
        }
    );
}

} // namespace conlog
