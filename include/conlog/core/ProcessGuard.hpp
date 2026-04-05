#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>

namespace conlog {

class ProcessGuard {
public:
    using Callback = std::function<void()>;

    explicit ProcessGuard(std::string_view process_name);

    [[nodiscard]] std::optional<int> wait_for_process(std::stop_token st);

    void monitor_async(int pid, Callback on_exit, std::stop_token st);

    [[nodiscard]] bool is_running(int pid) const noexcept;

private:
    [[nodiscard]] std::optional<int> scan_proc() const;

    std::string m_process_name;  // truncated to 15 chars (kernel comm limit)
    std::jthread m_monitor_thread;
    static constexpr std::chrono::milliseconds k_poll_interval{500};
};

} // namespace conlog
