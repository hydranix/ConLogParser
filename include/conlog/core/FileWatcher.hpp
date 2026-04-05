#pragma once

#include <expected>
#include <filesystem>
#include <functional>
#include <stop_token>
#include <string>
#include <thread>

namespace conlog {

class FileWatcher {
public:
    using NewDataCallback = std::function<void()>;

    explicit FileWatcher(std::filesystem::path log_path);
    ~FileWatcher();

    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    [[nodiscard]] std::expected<void, std::string> start(NewDataCallback cb, std::stop_source& ss);

    void stop() noexcept;

private:
    void watch_loop(std::stop_token jthread_st, std::stop_token ext_st,
                    NewDataCallback cb);

    [[nodiscard]] std::expected<void, std::string> open_watch();

    void close_watch() noexcept;

    void close_inotify() noexcept;
    [[nodiscard]] std::expected<void, std::string> open_inotify();

    std::filesystem::path m_log_path;
    int m_inotify_fd{-1};
    int m_watch_descriptor{-1};
    int m_event_fd{-1};       // opened once, never recycled in watch_loop
    std::jthread m_thread;
};

} // namespace conlog
