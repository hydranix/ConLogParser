#pragma once

#include <regex>
#include <string_view>

namespace conlog::events { class EventBus; }

namespace conlog {

class PlayerConnectedParser {
public:
    explicit PlayerConnectedParser(events::EventBus& bus);

    void operator()(std::string_view line);

private:
    events::EventBus& m_bus;
    std::regex m_pattern;
};

} // namespace conlog
