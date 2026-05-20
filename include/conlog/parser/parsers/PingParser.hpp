#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace conlog {

// Temporary parser that captures the player roster from a "ping" command's
// output. The first name in the result is the local player. The owner should
// deregister this parser from the supplied callback once it fires.
class PingParser {
public:
    using DoneCallback = std::function<void(std::vector<std::string>)>;

    explicit PingParser(DoneCallback on_done);

    void operator()(std::string_view line);

private:
    DoneCallback m_on_done;
    std::regex   m_pattern;
    std::mutex   m_mutex;
    std::vector<std::string> m_names;
    bool m_collecting{false};
    std::atomic<bool> m_done{false};
};

} // namespace conlog
