#pragma once

#include <condition_variable>
#include <mutex>
#include <stop_token>

#include "conlog/events/EventBus.hpp"
#include "conlog/parser/ParserRegistry.hpp"
#include "conlog/core/ProcessGuard.hpp"
#include "conlog/core/LineBuffer.hpp"
#include "conlog/core/Dispatcher.hpp"
#include "conlog/core/FileWatcher.hpp"

namespace conlog {

class App {
public:
    App();

    [[nodiscard]] int run();

private:
    void register_default_parsers();
    void shutdown();

    // m_stop_source MUST be declared before m_process_guard, m_file_watcher
    // so that it outlives their threads during destruction.
    std::stop_source m_stop_source;

    events::EventBus m_event_bus;
    ParserRegistry   m_registry;
    ProcessGuard     m_process_guard;
    LineBuffer       m_line_buffer;
    Dispatcher       m_dispatcher;
    FileWatcher      m_file_watcher;

    std::mutex       m_shutdown_mutex;
    std::condition_variable m_shutdown_cv;

    // Mutex protecting stdout writes from parallel parser callbacks.
    std::mutex       m_print_mutex;
};

} // namespace conlog
