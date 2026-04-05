#include <conlog/App.hpp>
#include <conlog/parser/parsers/KillDeathParser.hpp>
#include <conlog/parser/parsers/DamageParser.hpp>
#include <conlog/parser/parsers/NoscopeParser.hpp>
#include <conlog/parser/parsers/StickyNadesParser.hpp>
#include <conlog/parser/parsers/SuicideParser.hpp>
#include <conlog/parser/parsers/PlayerConnectedParser.hpp>
#include <conlog/parser/parsers/ChatParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <csignal>
#include <memory>
#include <mutex>
#include <print>
#include <stop_token>
#include <thread>

namespace conlog {

// Async-signal-safe flag -- only sig_atomic_t + volatile is portable here.
static volatile std::sig_atomic_t g_signal_flag = 0;

App::App()
    : m_process_guard("cstrike_linux64")
    , m_line_buffer("/tmp/conlog.txt")
    , m_dispatcher(m_registry)
    , m_file_watcher("/tmp/conlog.txt")
{}

void App::register_default_parsers()
{
    m_registry.register_parser("kill_death",
        std::make_shared<KillDeathParser>(m_event_bus));
    m_registry.register_parser("damage",
        std::make_shared<DamageParser>(m_event_bus));
    m_registry.register_parser("noscope",
        std::make_shared<NoscopeParser>(m_event_bus));
    m_registry.register_parser("sticky_nades",
        std::make_shared<StickyNadesParser>(m_event_bus));
    m_registry.register_parser("suicide",
        std::make_shared<SuicideParser>(m_event_bus));
    m_registry.register_parser("player_connected",
        std::make_shared<PlayerConnectedParser>(m_event_bus));
    m_registry.register_parser("chat",
        std::make_shared<ChatParser>(m_event_bus));
}

int App::run()
{
    // Signal handlers only set a volatile sig_atomic_t -- fully async-signal-safe.
    std::signal(SIGINT, [](int) { g_signal_flag = 1; });
    std::signal(SIGTERM, [](int) { g_signal_flag = 1; });

    // Polling thread to translate the async-signal-safe flag into request_stop().
    std::jthread signal_poller([this](std::stop_token st) {
        while (!st.stop_requested()) {
            if (g_signal_flag) {
                m_stop_source.request_stop();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{50});
        }
    });

    // Hook stop_source to the condition variable so we wake up on stop.
    std::stop_callback stop_cb{m_stop_source.get_token(), [this] {
        m_shutdown_cv.notify_all();
    }};

    register_default_parsers();

    // Protect stdout from concurrent writes (parsers run in parallel).
    auto& print_mutex = m_print_mutex;

    (void)m_event_bus.subscribe<events::KillEvent>(
        [&print_mutex](const events::KillEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[KILL] {} killed {} with {}{}",
                         e.killer, e.victim, e.weapon,
                         e.headshot ? " (headshot)" : "");
        });
    (void)m_event_bus.subscribe<events::DamageEvent>(
        [&print_mutex](const events::DamageEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[DMG] {} {} \"{}\" - {} in {} hits",
                         e.given ? "Given to" : "Taken from",
                         e.given ? "→" : "←",
                         e.other_player, e.damage, e.hits);
        });
    (void)m_event_bus.subscribe<events::NoscopeEvent>(
        [&print_mutex](const events::NoscopeEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[NOSCOPE] {} noscoped{}!",
                         e.player,
                         e.headshot ? " + headshot" : "");
        });
    (void)m_event_bus.subscribe<events::StickyNadeStuckEvent>(
        [&print_mutex](const events::StickyNadeStuckEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[STICKY] {} stuck {}!",
                         e.thrower, e.victim);
        });
    (void)m_event_bus.subscribe<events::SuicideEvent>(
        [&print_mutex](const events::SuicideEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[SUICIDE] {}", e.player);
        });
    (void)m_event_bus.subscribe<events::PlayerConnectedEvent>(
        [&print_mutex](const events::PlayerConnectedEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[JOIN] {} connected", e.player);
        });
    (void)m_event_bus.subscribe<events::ChatMessageEvent>(
        [&print_mutex](const events::ChatMessageEvent& e) {
            std::lock_guard lock{print_mutex};
            std::println("[CHAT] {}: {}", e.player, e.message);
        });

    std::println("Waiting for cstrike_linux64...");

    auto pid = m_process_guard.wait_for_process(m_stop_source.get_token());
    if (!pid) {
        std::println("Stopped before process was found.");
        return 0;
    }

    std::println("cstrike_linux64 detected (pid {}). Watching /tmp/conlog.txt...", *pid);

    m_line_buffer.seek_to_end();

    auto result = m_file_watcher.start(
        [this] {
            m_line_buffer.drain([this](std::string_view line) {
                m_dispatcher.dispatch(line);
            });
        },
        m_stop_source
    );

    if (!result) {
        std::println(stderr, "FileWatcher error: {}", result.error());
        return 1;
    }

    m_process_guard.monitor_async(*pid,
        [this] {
            std::println("cstrike_linux64 exited. Shutting down.");
            m_stop_source.request_stop();
        },
        m_stop_source.get_token()
    );

    // Block until stop is requested.
    {
        std::unique_lock lock{m_shutdown_mutex};
        m_shutdown_cv.wait(lock, [this] {
            return m_stop_source.stop_requested();
        });
    }

    shutdown();
    return 0;
}

void App::shutdown()
{
    m_file_watcher.stop();
}

} // namespace conlog
