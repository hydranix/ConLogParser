#pragma once

#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "conlog/parser/ParserConcept.hpp"

namespace conlog {

class ParserRegistry {
public:
    template<Parser P>
    void register_parser(std::string name, std::shared_ptr<P> parser)
    {
        std::unique_lock lock{m_mutex};
        m_parsers.insert_or_assign(std::move(name), ParserHandle{std::move(parser)});
    }

    void deregister_parser(const std::string& name);

    [[nodiscard]] std::vector<ParserHandle> snapshot() const;

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, ParserHandle> m_parsers;
};

} // namespace conlog
