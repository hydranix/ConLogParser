#include <conlog/core/ProcessGuard.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

namespace conlog {

ProcessGuard::ProcessGuard(std::string_view process_name)
    : m_process_name(process_name.substr(0, 15))
{}

std::optional<int> ProcessGuard::scan_proc() const
{
    namespace fs = std::filesystem;
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator("/proc", ec)) {
        if (!entry.is_directory(ec)) continue;

        const auto& filename = entry.path().filename().string();
        if (filename.empty() || !std::all_of(filename.begin(), filename.end(),
                [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
        {
            continue;
        }

        try {
            auto comm_path = entry.path() / "comm";
            std::ifstream comm_file(comm_path);
            if (!comm_file.is_open()) continue;

            std::string comm;
            std::getline(comm_file, comm);

            if (!comm.empty() && comm.back() == '\n') {
                comm.pop_back();
            }

            if (comm == m_process_name) {
                return std::stoi(filename);
            }
        } catch (...) {
            // PID directory may have disappeared mid-iteration; skip it.
            continue;
        }
    }

    return std::nullopt;
}

std::optional<int> ProcessGuard::wait_for_process(std::stop_token st)
{
    while (!st.stop_requested()) {
        if (auto pid = scan_proc()) {
            return pid;
        }
        std::this_thread::sleep_for(k_poll_interval);
    }
    return std::nullopt;
}

bool ProcessGuard::is_running(int pid) const noexcept
{
    try {
        auto comm_path = std::filesystem::path("/proc") / std::to_string(pid) / "comm";
        std::ifstream comm_file(comm_path);
        if (!comm_file.is_open()) return false;

        std::string comm;
        std::getline(comm_file, comm);
        if (!comm.empty() && comm.back() == '\n') {
            comm.pop_back();
        }
        return comm == m_process_name;
    } catch (...) {
        return false;
    }
}

void ProcessGuard::monitor_async(int pid, Callback on_exit, std::stop_token st)
{
    m_monitor_thread = std::jthread(
        [this, pid, on_exit = std::move(on_exit), st](std::stop_token inner_st) {
            while (!inner_st.stop_requested() && !st.stop_requested()) {
                std::this_thread::sleep_for(k_poll_interval);
                if (!is_running(pid)) {
                    on_exit();
                    return;
                }
            }
        }
    );
}

} // namespace conlog
