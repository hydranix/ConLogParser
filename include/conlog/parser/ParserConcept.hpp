#pragma once

#include <functional>
#include <memory>
#include <string_view>

namespace conlog {

template<typename T>
concept Parser = requires(T p, std::string_view line) {
    { p(line) } -> std::same_as<void>;
};

class ParserHandle {
public:
    template<Parser P>
    explicit ParserHandle(std::shared_ptr<P> p)
        : m_invoke([p = std::move(p)](std::string_view line) { (*p)(line); })
    {}

    void invoke(std::string_view line) const { m_invoke(line); }

private:
    std::function<void(std::string_view)> m_invoke;
};

} // namespace conlog
