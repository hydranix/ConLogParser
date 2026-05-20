#pragma once

#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <string>
#include <vector>

#include <dyncmd/DynCmd.hpp>
#include <dyncmd/WindowChecker.hpp>

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
    [[nodiscard]] bool wait_for_window_focus();
    void issue_initial_ping();
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

    dyncmd::DynCmd        m_dyncmd;
    dyncmd::WindowChecker m_window_checker;

    std::mutex               m_roster_mutex;
    std::string              m_local_player;
    std::vector<std::string> m_player_roster;
};

} // namespace conlog
