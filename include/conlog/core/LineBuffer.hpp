#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

namespace conlog {

class LineBuffer {
public:
    using LineCallback = std::function<void(std::string_view)>;

    explicit LineBuffer(std::filesystem::path path);
    ~LineBuffer();

    LineBuffer(const LineBuffer&) = delete;
    LineBuffer& operator=(const LineBuffer&) = delete;

    void seek_to_end();

    void drain(LineCallback cb);

private:
    std::filesystem::path m_path;
    std::int64_t m_file_offset{0};
    std::string m_partial;
    int m_fd{-1};
};

} // namespace conlog
