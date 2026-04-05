#include <conlog/parser/ParserRegistry.hpp>

namespace conlog {

void ParserRegistry::deregister_parser(const std::string& name)
{
    std::unique_lock lock{m_mutex};
    m_parsers.erase(name);
}

[[nodiscard]] std::vector<ParserHandle> ParserRegistry::snapshot() const
{
    std::shared_lock lock{m_mutex};
    std::vector<ParserHandle> result;
    result.reserve(m_parsers.size());
    for (const auto& [name, handle] : m_parsers) {
        result.push_back(handle);
    }
    return result;
}

} // namespace conlog
