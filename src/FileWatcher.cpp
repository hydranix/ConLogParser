#include <conlog/core/FileWatcher.hpp>

#include <sys/inotify.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>

namespace conlog {

FileWatcher::FileWatcher(std::filesystem::path log_path)
    : m_log_path(std::move(log_path))
{}

FileWatcher::~FileWatcher()
{
    stop();
    close_watch();
}

// Opens only the inotify fd + watch descriptor (not the eventfd).
std::expected<void, std::string> FileWatcher::open_inotify()
{
    m_inotify_fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    if (m_inotify_fd < 0) {
        return std::unexpected(std::string("inotify_init1: ") + std::strerror(errno));
    }

    m_watch_descriptor = inotify_add_watch(
        m_inotify_fd,
        m_log_path.c_str(),
        IN_MODIFY | IN_MOVE_SELF | IN_DELETE_SELF
    );
    if (m_watch_descriptor < 0) {
        auto err = std::string("inotify_add_watch: ") + std::strerror(errno);
        ::close(m_inotify_fd);
        m_inotify_fd = -1;
        return std::unexpected(err);
    }

    return {};
}

// Closes only the inotify fd + watch descriptor (not the eventfd).
void FileWatcher::close_inotify() noexcept
{
    if (m_watch_descriptor != -1 && m_inotify_fd != -1) {
        inotify_rm_watch(m_inotify_fd, m_watch_descriptor);
        m_watch_descriptor = -1;
    }
    if (m_inotify_fd != -1) {
        ::close(m_inotify_fd);
        m_inotify_fd = -1;
    }
}

std::expected<void, std::string> FileWatcher::open_watch()
{
    auto result = open_inotify();
    if (!result) {
        return result;
    }

    m_event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (m_event_fd < 0) {
        auto err = std::string("eventfd: ") + std::strerror(errno);
        close_inotify();
        return std::unexpected(err);
    }

    return {};
}

void FileWatcher::close_watch() noexcept
{
    close_inotify();
    if (m_event_fd != -1) {
        ::close(m_event_fd);
        m_event_fd = -1;
    }
}

std::expected<void, std::string> FileWatcher::start(NewDataCallback cb, std::stop_source& ss)
{
    auto result = open_watch();
    if (!result) {
        return result;
    }

    m_thread = std::jthread(
        [this, cb = std::move(cb), ext_st = ss.get_token()](std::stop_token jthread_st) {
            watch_loop(jthread_st, ext_st, cb);
        }
    );

    return {};
}

void FileWatcher::stop() noexcept
{
    // Signal the eventfd to wake the poll loop.
    // m_event_fd is never recycled by watch_loop, so this is safe.
    if (m_event_fd != -1) {
        std::uint64_t val = 1;
        [[maybe_unused]] auto ret = ::write(m_event_fd, &val, sizeof(val));
    }
    if (m_thread.joinable()) {
        m_thread.request_stop();
        m_thread.join();
    }
}

void FileWatcher::watch_loop(std::stop_token jthread_st, std::stop_token ext_st,
                              NewDataCallback cb)
{
    alignas(alignof(inotify_event)) char buf[4096 + sizeof(inotify_event)];

    auto should_stop = [&] {
        return jthread_st.stop_requested() || ext_st.stop_requested();
    };

    while (!should_stop()) {
        pollfd fds[2] = {
            {m_inotify_fd, POLLIN, 0},
            {m_event_fd,   POLLIN, 0}
        };

        int ret = ::poll(fds, 2, -1);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (fds[1].revents & POLLIN) {
            break; // eventfd signaled -- stop
        }

        if (fds[0].revents & POLLIN) {
            ssize_t n = ::read(m_inotify_fd, buf, sizeof(buf));
            if (n <= 0) continue;

            for (ssize_t i = 0; i < n; ) {
                auto* ev = reinterpret_cast<inotify_event*>(buf + i);

                if (ev->mask & IN_MODIFY) {
                    cb();
                }

                if (ev->mask & (IN_MOVE_SELF | IN_DELETE_SELF)) {
                    // Only recycle the inotify fd, NOT the eventfd.
                    close_inotify();
                    std::this_thread::sleep_for(std::chrono::milliseconds{200});
                    auto reopen = open_inotify();
                    if (!reopen) {
                        return; // cannot re-establish watch; exit loop
                    }
                    break; // break inner loop to re-enter poll with new inotify fd
                }

                i += static_cast<ssize_t>(sizeof(inotify_event) + ev->len);
            }
        }
    }
}

} // namespace conlog
