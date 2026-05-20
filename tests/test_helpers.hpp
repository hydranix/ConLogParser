#pragma once

#include <print>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

namespace test {

struct TestCase {
    std::string name;
    void (*fn)();
};

inline std::vector<TestCase>& registry()
{
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* name, void (*fn)())
    {
        registry().push_back({name, fn});
    }
};

inline int g_current_failed = 0;

inline void fail(std::string_view msg,
                 std::source_location loc = std::source_location::current())
{
    std::println(stderr, "  FAIL {}:{}: {}", loc.file_name(), loc.line(), msg);
    ++g_current_failed;
}

#define CHECK(cond) do {                                                  \
    if (!(cond)) ::test::fail("CHECK failed: " #cond);                    \
} while (0)

#define CHECK_EQ(a, b) do {                                               \
    if (!((a) == (b))) ::test::fail("CHECK_EQ failed: " #a " == " #b);    \
} while (0)

#define CHECK_GE(a, b) do {                                               \
    if (!((a) >= (b))) ::test::fail("CHECK_GE failed: " #a " >= " #b);    \
} while (0)

#define TEST(name)                                                        \
    static void test_##name();                                            \
    static ::test::Registrar reg_##name(#name, test_##name);              \
    static void test_##name()

inline int run_all()
{
    int total = 0;
    int failed = 0;
    for (auto& tc : registry()) {
        g_current_failed = 0;
        std::println("[ RUN  ] {}", tc.name);
        tc.fn();
        ++total;
        if (g_current_failed > 0) {
            std::println("[ FAIL ] {}", tc.name);
            ++failed;
        } else {
            std::println("[  OK  ] {}", tc.name);
        }
    }
    std::println("");
    std::println("{} test(s), {} failed", total, failed);
    return failed == 0 ? 0 : 1;
}

} // namespace test
