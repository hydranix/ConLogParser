#include <conlog/parser/parsers/PingParser.hpp>

#include <regex>
#include <string>
#include <utility>

namespace conlog {

PingParser::PingParser(DoneCallback on_done)
    : m_on_done(std::move(on_done))
    , m_pattern(R"(^[0-9]+ ms : (.*)$)")
{}

void PingParser::operator()(std::string_view line)
{
    if (m_done.load(std::memory_order_acquire)) return;

    std::unique_lock lock{m_mutex};
    if (m_done.load(std::memory_order_relaxed)) return;

    if (!m_collecting) {
        if (line == "Client ping times:") {
            m_collecting = true;
        }
        return;
    }

    std::string s{line};
    std::smatch m;
    if (std::regex_match(s, m, m_pattern)) {
        m_names.push_back(m[1].str());
        return;
    }

    // First non-matching line after the header terminates the table.
    m_done.store(true, std::memory_order_release);
    auto names = std::move(m_names);
    auto cb    = std::move(m_on_done);
    lock.unlock();

    if (cb) cb(std::move(names));
}

} // namespace conlog
