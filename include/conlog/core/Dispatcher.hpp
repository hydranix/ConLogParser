#pragma once

#include <string_view>

#include "conlog/parser/ParserRegistry.hpp"

namespace conlog {

class Dispatcher {
public:
    explicit Dispatcher(ParserRegistry& registry);

    void dispatch(std::string_view line);

private:
    ParserRegistry& m_registry;
};

} // namespace conlog
