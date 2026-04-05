#include <conlog/core/LineBuffer.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace conlog {

LineBuffer::LineBuffer(std::filesystem::path path)
    : m_path(std::move(path))
{}

LineBuffer::~LineBuffer()
{
    if (m_fd != -1) {
        ::close(m_fd);
    }
}

void LineBuffer::seek_to_end()
{
    // Close existing fd to avoid a leak if called more than once.
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
    }

    m_fd = ::open(m_path.c_str(), O_RDONLY | O_CLOEXEC);
    if (m_fd < 0) {
        // File may not exist yet; that is fine, drain() will try to lazy-open.
        m_fd = -1;
        return;
    }
    m_file_offset = ::lseek(m_fd, 0, SEEK_END);
    if (m_file_offset < 0) {
        m_file_offset = 0;
    }
}

void LineBuffer::drain(LineCallback cb)
{
    // Lazy open if the file was not available at seek_to_end time.
    if (m_fd == -1) {
        m_fd = ::open(m_path.c_str(), O_RDONLY | O_CLOEXEC);
        if (m_fd < 0) {
            m_fd = -1;
            return;
        }
        m_file_offset = 0;
    }

    // Detect truncation: if file is now smaller than our offset, reset.
    struct stat st;
    if (::fstat(m_fd, &st) < 0) {
        return;
    }
    if (st.st_size < m_file_offset) {
        m_file_offset = 0;
        m_partial.clear();
    }

    // Read new data in chunks.
    char buf[4096];
    for (;;) {
        ssize_t n = ::pread(m_fd, buf, sizeof(buf), m_file_offset);
        if (n <= 0) break;

        m_file_offset += n;
        m_partial.append(buf, static_cast<std::string::size_type>(n));
    }

    // Extract complete lines from m_partial without O(n^2) erase-from-front.
    std::string::size_type start = 0;
    while (start < m_partial.size()) {
        auto pos = m_partial.find('\n', start);
        if (pos == std::string::npos) break;

        std::string_view line{m_partial.data() + start, pos - start};
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        cb(line);
        start = pos + 1;
    }

    // Keep only the remaining partial line.
    if (start > 0) {
        m_partial.erase(0, start);
    }
}

} // namespace conlog
